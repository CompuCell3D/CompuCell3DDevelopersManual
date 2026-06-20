Potts3D
=======

``Potts3D`` is the C++ object that performs the Cellular Potts
Model part of a CompuCell3D simulation. The implementation is in
``CompuCell3D/Potts3D/Potts3D.h`` and ``CompuCell3D/Potts3D/Potts3D.cpp``.

At a high level, ``Potts3D`` owns the cell lattice, owns the objects
that evaluate pixel-copy energy changes, and executes the Metropolis
copy attempts that make up each Monte Carlo Step (MCS). It does not own
the entire simulation. The ``Simulator`` owns the simulation lifecycle
and calls ``Potts3D`` once per MCS.

The most important idea is that ``Potts3D`` works with pointers to
cells, not cell objects stored directly in the lattice. A lattice site
contains either ``nullptr`` for medium or a ``CellG*`` pointing to the
cell occupying that site. A single biological cell usually occupies many
lattice sites, but all of those sites point to the same ``CellG`` object.

Main Responsibilities
---------------------

``Potts3D`` is responsible for the following tasks:

* Creating and resizing the cell field.
* Creating and destroying ``CellG`` objects.
* Maintaining the cell inventory.
* Registering objects that react to cell-field changes.
* Registering energy functions used during pixel-copy attempts.
* Running Metropolis copy attempts.
* Applying accepted pixel copies to the lattice.
* Calling low-level Potts steppers after copy attempts.

The main members that support these responsibilities are:

.. code-block:: cpp

    WatchableField3D<CellG *> *cellFieldG;
    EnergyFunctionCalculator *energyCalculator;
    CellInventory cellInventory;
    BasicClassGroupFactory cellFactoryGroup;
    std::vector<Stepper *> steppers;
    std::vector<FixedStepper *> fixedSteppers;
    Automaton *automaton;
    FluctuationAmplitudeFunction *fluctAmplFcn;

These members show the main design of the Potts layer. The lattice is a
``WatchableField3D<CellG*>``. Energy terms are not hard-coded into
``Potts3D``; they are registered as ``EnergyFunction`` objects and
summed by ``EnergyFunctionCalculator``. Cell attributes can be extended
without changing ``CellG`` by registering accessors with
``cellFactoryGroup`` and by using ``AttributeAdder`` objects.

The Cell Field
--------------

The cell field is created in ``Potts3D::createCellField``:

.. code-block:: cpp

    void Potts3D::createCellField(const Dim3D dim) {
        if (cellFieldG)
            throw CC3DException("createCellField() cell field G already created!");
        cellFieldG = new WatchableField3D<CellG *>(dim, 0);
    }

The type is important: ``WatchableField3D<CellG*>`` stores pointers to
``CellG`` objects. Assigning a new value to a lattice site changes only
the pointer stored at that site. It does not copy the cell object.

This is why a cell can occupy many pixels efficiently. The field stores
many references to one ``CellG`` instance instead of duplicating that
cell's data at every pixel.

WatchableField3D and Lattice Monitors
-------------------------------------

``WatchableField3D`` extends ``Field3DImpl`` by adding change watchers.
Whenever code calls ``set`` on the field, the field stores the new value
and then notifies every registered watcher:

.. code-block:: cpp

    virtual void set(const Point3D &pt, const T value) {
        T oldValue = Field3DImpl<T>::get(pt);
        Field3DImpl<T>::set(pt, value);

        for (unsigned int i = 0; i < changeWatchers.size(); i++)
            changeWatchers[i]->field3DChange(pt, value, oldValue);
    }

For the cell field, ``T`` is ``CellG*``. A watcher therefore receives:

* ``pt``: the lattice point that changed.
* ``value``: the new ``CellG*`` stored at that point.
* ``oldValue``: the previous ``CellG*`` stored at that point.

This observer pattern is central to CC3D. It lets plugins update derived
cell state automatically when the lattice changes. For example, a volume
tracker can increase the volume of the cell that gains a pixel and
decrease the volume of the cell that loses a pixel. Other watchers can
update center of mass, surface information, inertia tensors, neighbor
relationships, or plugin-specific attributes.

``Potts3D`` exposes watcher registration through:

.. code-block:: cpp

    void Potts3D::registerCellGChangeWatcher(CellGChangeWatcher *_watcher) {
        if (!_watcher)
            throw CC3DException("registerBCGChangeWatcher() _watcher cannot be NULL!");

        cellFieldG->addChangeWatcher(_watcher);
    }

The order of watcher registration matters because watchers are called in
registration order. CC3D may load some internal modules before others to
keep dependent cell attributes consistent.

CellG and Dynamic Cell Attributes
---------------------------------

``CellG`` is the core C++ cell object. It contains common fields such as
``id``, ``clusterId``, ``type``, ``volume``, center-of-mass components,
surface-related values, fluctuation amplitude, and several other common
attributes.

However, ``CellG`` is intentionally not a giant structure containing
every possible plugin attribute. Many attributes are added dynamically by
plugins. This keeps the base cell object small and allows new plugins to
attach their own data without editing ``CellG`` itself.

Two mechanisms are especially important:

* ``BasicClassGroupFactory cellFactoryGroup`` stores factories/accessors
  for extra per-cell data.
* ``AttributeAdder`` hooks into cell inventory events so attributes can
  be created when a cell is added and destroyed when a cell is removed.

``Potts3D::registerAttributeAdder`` connects an ``AttributeAdder`` to the
cell inventory:

.. code-block:: cpp

    void Potts3D::registerAttributeAdder(AttributeAdder *_attrAdder) {
        attrAdder = _attrAdder;
        cellInventory.registerWatcher(attrAdder->getInventoryWatcher());
    }

The corresponding watcher calls ``addAttribute(cell)`` when a cell is
added and ``destroyAttribute(cell)`` when a cell is removed. This is why
cell creation and destruction in CC3D involve more than calling ``new``
and ``delete``.

Cell Creation and Inventory
---------------------------

``Potts3D`` creates cells and inserts them into the simulation's cell
inventory. The lattice and the inventory serve different purposes:

* The lattice answers: which cell occupies this pixel?
* The inventory answers: which cells exist in the simulation?

This distinction matters for performance. Iterating over every cell
should not require scanning every lattice site. Python code such as:

.. code-block:: python

    for cell in self.cell_list:
        ...

ultimately relies on the C++ cell inventory rather than a full lattice
scan.

Creating a cell at a point follows this pattern:

.. code-block:: cpp

    CellG *Potts3D::createCellG(const Point3D pt, long _clusterId) {
        if (!cellFieldG->isValid(pt))
            throw CC3DException("createCell() cellFieldG Point out of range!");

        CellG *cell = createCell(_clusterId);
        cellFieldG->set(pt, cell);
        return cell;
    }

The call to ``cellFieldG->set`` is important. It triggers the registered
field watchers, so the new cell's volume and other tracked attributes can
be initialized consistently with the lattice.

Potts Energy Functions
----------------------

The Cellular Potts Model accepts or rejects a proposed pixel copy based
on the change in effective energy. In CC3D each energy term is
implemented as an object derived from ``EnergyFunction``:

.. code-block:: cpp

    class EnergyFunction {
    public:
        virtual double changeEnergy(
            const Point3D &pt,
            const CellG *newCell,
            const CellG *oldCell
        ) {
            return 0.0;
        }

        virtual std::string toString() {
            return std::string("EnergyFunction");
        }
    };

The arguments describe the proposed copy:

* ``pt`` is the lattice point that may change.
* ``newCell`` is the cell that would occupy ``pt`` if the copy is accepted.
* ``oldCell`` is the cell currently occupying ``pt``.

Plugins that contribute to the Potts energy register their energy
function with ``Potts3D``:

.. code-block:: cpp

    void Potts3D::registerEnergyFunctionWithName(
        EnergyFunction *_function,
        std::string _functionName
    ) {
        energyCalculator->registerEnergyFunctionWithName(_function, _functionName);
    }

``EnergyFunctionCalculator`` owns the summation loop. Its
``changeEnergy`` method calls every registered energy function and sums
the returned values:

.. code-block:: cpp

    double EnergyFunctionCalculator::changeEnergy(
        Point3D &pt,
        const CellG *newCell,
        const CellG *oldCell,
        const unsigned int _flipAttempt
    ) {
        double change = 0;
        for (unsigned int i = 0; i < energyFunctions.size(); i++) {
            change += energyFunctions[i]->changeEnergy(pt, newCell, oldCell);
        }
        return change;
    }

This indirection lets CC3D swap in specialized calculators, such as
statistics or test-data calculators, without changing the Metropolis
algorithm itself.

One Pixel-Copy Attempt
----------------------

A single Potts copy attempt in ``metropolisFast`` follows this logic:

1. Pick a random lattice point ``pt``.
2. Read the cell currently at ``pt``. This is the source cell.
3. Pick a neighbor of ``pt`` using ``BoundaryStrategy``.
4. Read the cell currently at the neighbor point. This is the target cell.
5. Skip the attempt if source and target are the same cell.
6. Skip the attempt if either cell type is frozen.
7. Ask ``EnergyFunctionCalculator`` for the total energy change.
8. Ask the fluctuation amplitude function for the effective motility.
9. Ask the acceptance function for the probability of accepting the copy.
10. If accepted, assign the source cell pointer to the target pixel.
11. Let field watchers and Potts steppers update dependent state.

The key lines in ``metropolisFast`` are:

.. code-block:: cpp

    CellG *cell = cellFieldG->getQuick(pt);
    Neighbor n = boundaryStrategy->getNeighborDirectVoxelCopy(pt, directIdx);
    Point3D changePixel = n.pt;
    CellG *changePixelCell = cellFieldG->getQuick(changePixel);

    double change = energyCalculator->changeEnergy(
        changePixel,
        cell,
        changePixelCell,
        i
    );

    double motility = fluctAmplFcn->fluctuationAmplitude(cell, changePixelCell);
    double prob = acceptanceFunction->accept(motility, change);

    if (prob >= 1.0 || rand->getRatio() < prob) {
        cellFieldG->set(changePixel, flipNeighborVec[currentWorkNodeNumber], cell);
    }

The copy direction is easy to confuse. The source cell is read from
``pt``. The target pixel is ``changePixel``. If the copy is accepted,
``changePixel`` is overwritten with the source cell pointer.

BoundaryStrategy
----------------

``Potts3D`` delegates neighbor lookup and boundary-condition behavior to
``BoundaryStrategy``. This keeps the Metropolis code independent of the
details of square versus hexagonal lattices, periodic boundaries,
no-flux boundaries, neighbor order, and maximum neighbor distance.

The simulator instantiates ``BoundaryStrategy`` while reading the
``<Potts>`` section, after dimensions, lattice type, dimension type, and
boundary-condition tags have been parsed.

Acceptance and Fluctuation Amplitude
------------------------------------

After the energy change has been computed, ``Potts3D`` asks two objects
to decide whether the copy should be accepted:

* ``FluctuationAmplitudeFunction`` computes the effective fluctuation
  amplitude for the two cells involved in the copy.
* ``AcceptanceFunction`` converts fluctuation amplitude and energy change
  into an acceptance probability.

The default acceptance behavior is the usual Potts behavior: energetically
favorable copies are accepted, while unfavorable copies may still be
accepted with a probability that decreases as the energy increase becomes
large relative to the fluctuation amplitude.

The fluctuation amplitude may be global or cell-type-specific. When two
cells are involved, CC3D can combine their amplitudes using strategies
such as minimum, maximum, or arithmetic average. These strategies are
implemented in ``StandardFluctuationAmplitudeFunctions``.

Steppers and Fixed Steppers
---------------------------

``Potts3D`` has two low-level callback mechanisms that are separate from
simulation-level steppables:

* ``FixedStepper`` objects run before every copy attempt.
* ``Stepper`` objects run after copy attempts.

The ``Stepper`` interface is intentionally small:

.. code-block:: cpp

    class Stepper {
    public:
        virtual void step() = 0;
    };

These callbacks are for low-level Potts maintenance tasks that must be
synchronized with pixel-copy attempts. They are not the same as
``Steppable`` modules managed by ``Simulator`` and ``ClassRegistry``.
Simulation-level steppables run once per MCS, after ``Potts3D`` finishes
all copy attempts for that MCS.

How Plugins Usually Interact with Potts3D
-----------------------------------------

A C++ plugin can interact with ``Potts3D`` in several ways:

* Register an ``EnergyFunction`` to contribute to copy-attempt energy.
* Register a ``CellGChangeWatcher`` to react when the lattice changes.
* Register a ``Stepper`` or ``FixedStepper`` for low-level copy-attempt
  callbacks.
* Register per-cell attribute accessors or an ``AttributeAdder`` for
  plugin-specific cell data.
* Query the cell field, cell inventory, automaton, or boundary strategy.

Most plugins use more than one of these mechanisms. For example, a
constraint plugin may register an energy function and also register a
watcher to keep cached cell attributes up to date.

Relationship to Simulator
-------------------------

``Potts3D`` does not decide when the simulation starts or stops. The
``Simulator`` calls ``potts.metropolis`` from ``Simulator::step``. The
number of copy attempts is computed from the lattice volume and
``Flip2DimRatio``:

.. code-block:: cpp

    Dim3D dim = potts.getCellFieldG()->getDim();
    int flipAttempts = (int)(dim.x * dim.y * dim.z * ppdCC3DPtr->flip2DimRatio);
    int flips = potts.metropolis(flipAttempts, ppdCC3DPtr->temperature);

After ``potts.metropolis`` returns, the simulator calls simulation-level
steppables through ``ClassRegistry``. This separation is useful when
reading CC3D C++ code:

* ``Potts3D`` handles pixel-copy mechanics inside one MCS.
* ``Simulator`` handles the lifecycle and the per-MCS scheduling of
  steppables.
