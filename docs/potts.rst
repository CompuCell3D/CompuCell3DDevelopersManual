Potts3D
---------

Potts3D module (``Potts3D/Potts3D.cpp``, ``Potts3D/Potts3D.h``) implements entire logic of the Potts algorithm. Moreover,
this module is responsible for creating cell lattice and ``Potts3D`` class has methods that facilitate creation and
destruction of cells. It is worth pointing out that creation and destruction of cells is not limited to calling
``new`` or ``delete`` operators but it also involves several steps necessary to ensure that cells created have all the
attributes needed by requested by the user plugins. In CC3D cells' attributes are added dynamically
and CC3D cells by default have only a small subset of attributes hard-coded. This is a design decisiopn that has this nice
consequence that when developing new plugin one does not have to modify ``CellG`` class but rather program the addition
of cell's attributes entirely in the plugins code. We will cover this in detail in later section.

Let's examine the content of the ``Potts3D`` class (**Note:** we removed some of the code and are presenting only
code snippets most relevant to current discussion. You are encouraged to look at the original code though as you go over
the material presented here):

.. code-block:: cpp

	class Potts3D :public SteerableObject {
		WatchableField3D<CellG *> *cellFieldG;
		AttributeAdder * attrAdder;
		EnergyFunctionCalculator * energyCalculator;

		BasicClassGroupFactory cellFactoryGroup; 	//creates aggregate of objects associated with cell


		/// An array of energy functions to be evaluated to determine energy costs.
		std::vector<EnergyFunction *> energyFunctions;
		EnergyFunction * connectivityConstraint;

		std::map<std::string, EnergyFunction *> nameToEnergyFuctionMap;

        ...

		std::vector<BasicRandomNumberGeneratorNonStatic> randNSVec;

		/// An array of potts steppers.  These are called after each potts step.
		std::vector<Stepper *> steppers;

		std::vector<FixedStepper *> fixedSteppers;
		/// The automaton to use.  Assuming one automaton per simulation.
		Automaton* automaton;

        ...

		FluctuationAmplitudeFunction * fluctAmplFcn;

		/// The current total energy of the system.
		double energy;

		std::string boundary_x; // boundary condition for x axiz
		std::string boundary_y; // boundary condition for y axis
		std::string boundary_z; // boundary condition for z axis

		/// This object keeps track of all cells available in the simulations. It allows for simple iteration over all the cells
		/// It becomes useful whenever one has to visit all the cells. Without inventory one would need to go pixel-by-pixel - very inefficient
		CellInventory cellInventory;

		Point3D flipNeighbor;
		std::vector<Point3D> flipNeighborVec; //for parallel access

		double depth;
		//int maxNeighborOrder;
		std::vector<Point3D> neighbors;
		std::vector<unsigned char> frozenTypeVec;///lists types which will remain frozen throughout the simulation
		unsigned int sizeFrozenTypeVec;

		ParallelUtilsOpenMP *pUtils;

	public:

		Potts3D();
		Potts3D(const Dim3D dim);
		virtual ~Potts3D();

		void createCellField(const Dim3D dim);
		void resizeCellField(const Dim3D dim, Dim3D shiftVec = Dim3D());

		double getTemperature() const { return temperature; }

		void registerConnectivityConstraint(EnergyFunction * _connectivityConstraint);
		EnergyFunction * getConnectivityConstraint();

		bool checkIfFrozen(unsigned char _type);

        ...

		void initializeCellTypeMotility(std::vector<CellTypeMotilityData> & _cellTypeMotilityVector);
		void setCellTypeMotilityVec(std::vector<float> & _cellTypeMotilityVec);
		const std::vector<float> & getCellTypeMotilityVec() const { return cellTypeMotilityVec; }

		void setDebugOutputFrequency(unsigned int _freq) { debugOutputFrequency = _freq; }
		void setSimulator(Simulator *_sim) { sim = _sim; }

        ...

		Point3D getFlipNeighbor();

        ...

		virtual void createEnergyFunction(std::string _energyFunctionType);
		EnergyFunctionCalculator * getEnergyFunctionCalculator() { return energyCalculator; }

		CellInventory &getCellInventory() { return cellInventory; }

		void clean_cell_field(bool reset_cell_inventory = true);

		virtual void registerAttributeAdder(AttributeAdder * _attrAdder);
		virtual void registerEnergyFunction(EnergyFunction *function);
		virtual void registerEnergyFunctionWithName(EnergyFunction *_function, std::string _functionName);
		virtual void unregisterEnergyFunction(std::string _functionName);

		/// Add the automaton.
		virtual void registerAutomaton(Automaton* autom);

		/// Return the automaton for this simulation.
		virtual Automaton* getAutomaton();
		void setParallelUtils(ParallelUtilsOpenMP *_pUtils) { pUtils = _pUtils; }

		virtual void setFluctuationAmplitudeFunctionByName(std::string _fluctuationAmplitudeFunctionName);
		/// Add a cell field update watcher.

		/// registration of the BCG watcher
		virtual void registerCellGChangeWatcher(CellGChangeWatcher *_watcher);

		/// Register accessor to a class with a cellGroupFactory. Accessor will access a class which is a mamber of a BasicClassGroup
		virtual void registerClassAccessor(BasicClassAccessorBase *_accessor);

		/// Add a potts stepper to be called after each potts step.
		virtual void registerStepper(Stepper *stepper);
		virtual void registerFixedStepper(FixedStepper *fixedStepper, bool _front = false);
		virtual void unregisterFixedStepper(FixedStepper *fixedStepper);

		double getEnergy();

		virtual CellG *createCellG(const Point3D pt, long _clusterId = -1);
		virtual CellG *createCellGSpecifiedIds(const Point3D pt, long _cellId, long _clusterId = -1);
		virtual CellG *createCell(long _clusterId = -1);
		virtual CellG *createCellSpecifiedIds(long _cellId, long _clusterId = -1);

		virtual void destroyCellG(CellG * cell, bool _removeFromInventory = true);

		BasicClassGroupFactory * getCellFactoryGroupPtr() { return &cellFactoryGroup; };

		virtual unsigned int getNumCells() { return cellInventory.getCellInventorySize(); }

		virtual double changeEnergy(Point3D pt, const CellG *newCell,const CellG *oldCell);

		virtual unsigned int metropolis(const unsigned int steps,const double temp);

		typedef unsigned int (Potts3D::*metropolisFcnPtr_t)(const unsigned int, const double);

		metropolisFcnPtr_t metropolisFcnPtr;

		unsigned int metropolisList(const unsigned int steps, const double temp);

		unsigned int metropolisFast(const unsigned int steps, const double temp);
		unsigned int metropolisBoundaryWalker(const unsigned int steps, const double temp);
		void setMetropolisAlgorithm(std::string _algName);

		virtual Field3D<CellG *> *getCellFieldG() { return (Field3D<CellG *> *)cellFieldG; }
		virtual Field3DImpl<CellG *> *getCellFieldGImpl() { return (Field3DImpl<CellG *> *)cellFieldG; }

		//SteerableObject interface
		virtual void update(CC3DXMLElement *_xmlData, bool _fullInitFlag = false);
		virtual std::string steerableName();
		virtual void runSteppers();
		long getRecentlyCreatedClusterId() { return recentlyCreatedClusterId; }
		long getRecentlyCreatedCellId() { return recentlyCreatedCellId; }

	};


Starting from the top of the file we notice that cell lattice (``WatchableField3D<CellG *> *cellFieldG;``) is owned and
created by (``void createCellField(const Dim3D dim);``, ``void resizeCellField(const Dim3D dim, Dim3D shiftVec = Dim3D());``) ``Potts3D``.

The cell lattice is an instance of the ``WatchableField3D`` class (which strictly speaking is a template class).
The cell lattice stores **pointers** to cell objects (type ``CellG*``). This means that is I have a single cell but assign the pointer to it
to several lattice sites , I caused my single cell to have volume equal to the number of lattice sites that have pointer to my cell
This way ``CellG`` objects do not get repeated for every pixel (this woudl cost too much memory) but rather are referenced from the
cell lattice via pointers.
The reason cell lattice field is called "Watchable" is because this class implements the observer design pattern.
This means any, manipulation of the cell lattice (e.g. assigning cell to a given pixel) triggers calls to multiple registered
observer objects that react to such change. For example if I am extending a cell by assigning its pointer to the new lattice site
one of the observer that will be called (we also refere to them as lattice monitors) is a module that tracks cell volume
The cell that gains new pixel will get its ``volume`` attribute increased by 1 and the cell that loses one pixel will \
get its volume decreased by 1. Similarly we could have another observer that updates center of mass coordinates, or one that monitors
inertia tensor. The nice thing about using ``WatchableField3D`` template is that all those observers are called automatically
when change in the lattice takes place. Let's look at how this is done

WatchableField3D
~~~~~~~~~~~~~~~~

.. code-block:: cpp

    #ifndef WATCHABLEFIELD3D_H
    #define WATCHABLEFIELD3D_H

    #include "Field3DImpl.h"
    #include "Field3DChangeWatcher.h"

    #include <BasicUtils/BasicArray.h>
    #include <BasicUtils/BasicException.h>

    namespace CompuCell3D {

      template <class T>
      class Field3DImpl;

      template <class T>
      class WatchableField3D: public Field3DImpl<T> {
        BasicArray<Field3DChangeWatcher<T> *> changeWatchers;

      public:
        /**
         * @param dim The field dimensions
         * @param initialValue The initial value of all data elements in the field.
         */
        WatchableField3D(const Dim3D dim, const T &initialValue) :
          Field3DImpl<T>(dim, initialValue) {}

          virtual ~WatchableField3D(){}
        virtual void addChangeWatcher(Field3DChangeWatcher<T> *watcher) {
          ASSERT_OR_THROW("addChangeWatcher() watcher cannot be NULL!", watcher);
          changeWatchers.put(watcher);
        }

        virtual void set(const Point3D &pt, const T value) {
          T oldValue = Field3DImpl<T>::get(pt);
          Field3DImpl<T>::set(pt, value);

          for (unsigned int i = 0; i < changeWatchers.getSize(); i++)
        changeWatchers[i]->field3DChange(pt, value, oldValue);
        }
      };
    };
    #endif

The ``WatchableField3D<T>`` template class inherits from ``Field3DImpl<T>`` template. The actual memory allocation takes
place in the ``Field3DImpl<T>`` but we will not worry about it here. It is sufficient to mention that ``Field3DImpl<T>``
is tha class that manages cell lattice memory. The important thing is to understand how this automatic calling
of lattice monitors is implemented. The ``WatchableField3D<T>`` class has a container
``BasicArray<Field3DChangeWatcher<T> *> changeWatchers;`` that stores pointers to lattice monitors. The lattice monitor object
is a class that inherits ``Field3DChangeWatcher<T>`` class. In CC3D case ``T`` is set to ``CellG*``. The  ``BasicArray``
is a thin wrapper around ``std::vector`` class and it is one of the legacies of the early CC3D implementations. So
``WatchableField3D<T>`` class has a collection of objects that react to the changes in the cell lattice. How do they react?
If we look at the implementation of `` virtual void set(const Point3D &pt, const T value)`` function that modifies the lattice
we can see that this function fetches old value stored in the lattice at location indicated by ``Point3D pt`` - in the case of
cell lattice this will be pointers currently stored at this location. It then assigns new value to the field (new ``CellG`` pointer)
and then it calls all registered lattice monitors:

.. code-block:: cpp

      for (unsigned int i = 0; i < changeWatchers.getSize(); i++)
            changeWatchers[i]->field3DChange(pt, value, oldValue);

In particular each lattice monitor (here referred to as ``changeWatcher``) must define function called ``field3DChange``
that takes 3 arguments - location of the change ``pt``, new value we assign to the field (e.g. new pointer to ``CellG`` object)
and old value that was stored in the field before the assignment (e.g. pointer to the cell whose pixel gets overwritten).

This way the process of updating attributes of ``CellG`` object can be handled by appropriate ``changeWatchers``. We will
cover in detail examples of change watchers and things will become clearer then.

Energy Functions
~~~~~~~~~~~~~~~~

Few lines below declaration of ``cellField``, which as we know is an instance of  ``WatchableField3D<CellG *>``
we find the declaration of containers associated with Energy function calculations. At this point we remind that the essence
of Cellular Potts Model is in calculating change of energy opf the system due to randomly chosen lattice perturbation
(change of the single pixel). Pointers energy functions objects are stored inside ``Potts3D`` object as follows:

.. code-block:: cpp

    /// An array of energy functions to be evaluated to determine energy costs.
    std::vector<EnergyFunction *> energyFunctions;
    EnergyFunction * connectivityConstraint;

    std::map<std::string, EnergyFunction *> nameToEnergyFuctionMap;

All energy functions are actually objects and they all inherit base class ``EnergyFunction``. ``EnergyFunction`` is defined
inside ``Potts3D/EnergyFunction.h`` header file:

.. code-block:: cpp

	class EnergyFunction {

	public:
		EnergyFunction() {}
		virtual ~EnergyFunction() {}

		virtual double localEnergy(const Point3D &pt){return 0.0;};

		virtual double changeEnergy(const Point3D &pt, const CellG *newCell,const CellG *oldCell)
		{
			if(1!=1);return 0.0;
		}
		virtual std::string toString()
		{
			return std::string("EnergyFunction");
		}
	};

Each class that is responsible for calculating a change in the overall system energy due to a proposed pixel copy has to
inherit ``EnergyFunction``. The key function that has to be reimplemented in the derived class is
``virtual double changeEnergy(const Point3D &pt, const CellG *newCell,const CellG *oldCell)``. After metropolis algorithm
function picks candidate for pixel overwrite it will then call ``changeEnergy`` for every element of the ``energyFunctions`` vector
defined in class ``Potts3D`` (see above). The ``pt`` argument is a reference to a location of a pixel
(specified as simple object ``Point3D``) that would be overwritten as result of the pixel copy attempt. The ``newCell``
is pointer to a cell object that will occupy ``pt`` location of the ``cellField`` IF we accept pixel copy and the
``oldCell`` is a pointer to a cell that currently occupies lattice location ``pt``.

in CompuCell3D users declare which energy functions they want to use in their simulation so that the number of
energy function in the ``energyFunctions`` vector will vary depending on what users specify in the CC3DML or in Python.

Later we will present detailed information on how to implement energy function plugins.

When we peek at the ``metropolisFast`` function of the ``Potts3D`` class we can see that the change of energy is calculated
in a fairly straightforward way:

.. code-block:: cpp

        Point3D pt;

        // Pick a random point
        pt.x = rand->getInteger(sectionDims.first.x, sectionDims.second.x - 1);
        pt.y = rand->getInteger(sectionDims.first.y, sectionDims.second.y - 1);
        pt.z = rand->getInteger(sectionDims.first.z, sectionDims.second.z - 1);

        CellG *cell = cellFieldG->getQuick(pt);

        if (sizeFrozenTypeVec && cell) {///must also make sure that cell ptr is different 0; Will never freeze medium
            if (checkIfFrozen(cell->type))
                continue;
        }

        unsigned int directIdx = rand->getInteger(0, maxNeighborIndex);

        Neighbor n = boundaryStrategy->getNeighborDirect(pt, directIdx);

        if (!n.distance) {
            //if distance is 0 then the neighbor returned is invalid
            continue;
        }
        Point3D changePixel = n.pt;

        //check if changePixel refers to different cell.
        CellG* changePixelCell = cellFieldG->getQuick(changePixel);

        if (changePixelCell == cell) {
            //skip the rest of the loop if change pixel points to the same cell as pt
            continue;
        }

        if (sizeFrozenTypeVec && changePixelCell) {///must also make sure that cell ptr is different 0; Will never freeze medium
            if (checkIfFrozen(changePixelCell->type))
                continue;
        }

        ++attemptedECVec[currentWorkNodeNumber];

        flipNeighborVec[currentWorkNodeNumber] = pt;

        /// change takes place at change pixel  and pt is a neighbor of changePixel
        // Calculate change in energy

        double change = energyCalculator->changeEnergy(changePixel, cell, changePixelCell, i);

We first pick a random lattice location ``pt`` and retrieve pointer of a cell that occupies this location:

.. code-block:: cpp

    CellG *cell = cellFieldG->getQuick(pt);

We next make sure that the cell can move *i.e.* it is not frozen:

.. code-block:: cpp

    if (sizeFrozenTypeVec && cell) {///must also make sure that cell ptr is different 0; Will never freeze medium
        if (checkIfFrozen(cell->type))
            continue;
    }

Next we pick a random pixel out of set of neighbors of pixel ``pt``:

.. code-block::

    unsigned int directIdx = rand->getInteger(0, maxNeighborIndex);

    Neighbor n = boundaryStrategy->getNeighborDirect(pt, directIdx);

    if (!n.distance) {
        //if distance is 0 then the neighbor returned is invalid
        continue;
    }
    Point3D changePixel = n.pt;

    //check if changePixel refers to different cell.
    CellG* changePixelCell = cellFieldG->getQuick(changePixel);

    if (changePixelCell == cell) {
        //skip the rest of the loop if change pixel points to the same cell as pt
        continue;
    }

    if (sizeFrozenTypeVec && changePixelCell) {///must also make sure that cell ptr is different 0; Will never freeze medium
        if (checkIfFrozen(changePixelCell->type))
            continue;
    }
We use ``BoundaryStrategy`` object pointed by ``boundaryStrategy`` to carry out all operations related to pixel neighbor
operations. we will cover it later. For now it is important to remember that tracking.operating on pixel neighbors is
usually done via ``BoundaryStrategy`` and this heps greatly when we have to deal with periodic boundary conditions
pixels residing close to the edge of teh lattice or classifying neighbor order of pixels.
in this example we use boundary strategy to pick a neighbor ``changePixel`` of the ``pt`` and verify that this neighbor is a
legitimate neighbor - ``if (!n.distance)``. We next fetch cell that occupies ``changePixel``:

.. code-block:: cpp

    CellG* changePixelCell = cellFieldG->getQuick(changePixel);

and verify that ``changePixelCell`` is different than cell at the location ``pt``. We do this because overwriting pixel
with the same cell pointer does not change lattice configuration at all. After also confirming that the ``changePixelCell``
is not frozen we compute change of energy if pixel ```changePixel`` currently occupied by ``changePixelCell``
were to be overwritten by ``cell`` currently residing at location ``pt``. Or using ``double changeEnergy(const Point3D &pt, const CellG *newCell,const CellG *oldCell)``
terminology we can say that ``pt <-> changePixel``, ``newCell <-> cell`` and ``oldCell <-> changePixelCell`` where
we used ``<->`` symbol to illustrate how ``changeEnergy`` function arguments will be assigned in the call.

Interestingly, we call ``changeEnergy`` method of the object called ``energyCalculator``:

.. code-block:: cpp

    double change = energyCalculator->changeEnergy(changePixel, cell, changePixelCell, i);

There is no magic here. If we look inside this function (``Potts3D/EnergyFunctionCalculator.cpp``) we see
familiar summation over all values returned by ``changeEnergy`` of each ``EnergyFunction`` object:

.. code-block:: cpp

    double EnergyFunctionCalculator::changeEnergy(Point3D &pt, const CellG *newCell,const CellG *oldCell,const unsigned int _flipAttempt){

        double change = 0;
        for (unsigned int i = 0; i < energyFunctions.size(); i++){
            change += energyFunctions[i]->changeEnergy(pt, newCell, oldCell);
        }
        return change;
    }

The reason we use ``EnergyFunctionCalculator`` object instead of implementing summation loop inside ``metropolisFast`` function
is to handle additional tasks that might be associated with calculating energies - for example collecting information
on every energy term associated with every pixel copy attempts. In this case we would use not ``EnergyFunctionCalculator`` but
a more sophisticated version of this class called ``EnergyFunctionCalculatorStatistics``

Steppers
~~~~~~~~

A vector of ``Stepper`` objects - ``std::vector<Stepper *> steppers;`` is also a part of ``Potts3D`` object.
Stepper objects all inherit from ``Stepper`` class defined in ``Potts3D/Stepper.h`` header file:

.. code-block:: cpp

    class Stepper {
    public:
        virtual void step() = 0;
    };

This is a very simple base class that defines only one function called ``step``. More important is the question
where and why we need this function. Steppers are called at the very end of the pixel copy attempt *i.e.* after
all energy function calculation and if pixel copy was accpted after modifying ``cellField``. Steppers are called
always regardless whether pixel copy was accepted or not. A canonical example of the ``Stepper`` object is ``VolumeTracker``
declared and defined in ``plugins/VolumeTracker/VolumeTrackerPlugin.h`` and
``plugins/VolumeTracker/VolumeTrackerPlugin.cpp``. ``VolumeTracker`` plugin tracks volume of each cell and ensures that
cells' volume information is correct. It also removes dead cells i./e. those cells whose volume reached 0. In a sense it
performs cleanup actions. However cleanup needs to be done as a very last action associated with pixel copy attempt.
It would be a bad idea to do it earlier because we could remove cell object that might still be needed by other actions
related to *e.g.* updating ``cellField``.

Cell Inventory
~~~~~~~~~~~~~~

``cellInventory`` as its name suggest is an object that serves as a container for pointers to cell objects but it also
allows fast lookups of particular cells. This is one of he most frequently accessed objects from Python
(although we do it somewhat indirectly). Many of the Python modules you write for CC3D include the following loop:

.. code-block:: python

    for cell in self.cellList:
        ...

What we are doing here is we iterate over every cell in the simulation. Internally the ``self.sellList`` Python object
accesses ``cellInventory``. when we create a cell using ``Potts3D``'s method ``createCellG`` we first construct cell object
and then insert it into cell inventory. Similarly when we delete cell object using ``destroyCellG`` (method of ``Potts3D``)
we first remove the ``cell`` object from inventory and then carryout its destruction
(which as you know is not just simple call to  the C++ ``delete`` operator)

Acceptance Function and Fluctuation Amplitude Function
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

One of the key aspects
