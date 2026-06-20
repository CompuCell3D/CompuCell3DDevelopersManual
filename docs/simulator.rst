Simulator
=========

``Simulator`` is the top-level C++ object that coordinates a CompuCell3D
simulation. Its implementation is in ``CompuCell3D/Simulator.h`` and
``CompuCell3D/Simulator.cpp``.

The simulator does not implement the Potts algorithm directly. Instead,
it owns a ``Potts3D`` object and calls it once per Monte Carlo Step
(MCS). It also loads plugins and steppables, owns the class registry,
keeps simulation-wide parse data, manages concentration-field lookup,
and performs cleanup when a run finishes.

The shortest accurate summary is:

    ``Simulator`` reads the parsed CC3DML configuration, initializes
    ``Potts3D`` and dynamically loaded modules, then repeatedly calls
    ``Potts3D::metropolis`` followed by simulation-level steppables.

Important Members
-----------------

The core members in ``Simulator`` show its role as the owner of the run:

.. code-block:: cpp

    ClassRegistry *classRegistry;
    Potts3D potts;
    int currstep;
    std::map<std::string, Field3D<float>*> concentrationFieldNameMap;
    std::map<std::string, SteerableObject *> steerableObjectMap;
    PottsParseData *ppdCC3DPtr;
    ParallelUtilsOpenMP *pUtils;
    ParallelUtilsOpenMP *pUtilsSingle;

A few relationships are worth remembering:

* ``Simulator`` contains ``Potts3D`` directly, not through a pointer.
* ``ClassRegistry`` stores active simulation-level steppables.
* ``PottsParseData`` stores parsed ``<Potts>`` settings such as lattice
  dimensions, number of steps, temperature/fluctuation amplitude,
  ``Flip2DimRatio``, annealing steps, and boundary settings.
* ``concentrationFieldNameMap`` maps field names to concentration-field
  pointers so plugins and steppables can retrieve fields by name.
* ``steerableObjectMap`` stores objects that can be updated during
  steering.

Namespace and Export Macro
--------------------------

Most core CC3D classes are in the ``CompuCell3D`` namespace:

.. code-block:: cpp

    namespace CompuCell3D {
        class COMPUCELLLIB_EXPORT Simulator : public Steppable {
            ...
        };
    }

The ``COMPUCELLLIB_EXPORT`` macro handles platform-specific symbol export
and import rules. On Windows it expands to the appropriate
``__declspec`` decoration. On other platforms it is typically empty.
This is why many CC3D classes are declared with export macros even when
the macro does not affect Linux or macOS builds.

Simulation Lifecycle
--------------------

The simulator lifecycle is organized around these methods:

.. code-block:: cpp

    void initializeCC3D();
    void initializePottsCC3D(CC3DXMLElement *_xmlData);
    void processMetadataCC3D(CC3DXMLElement *_xmlData);
    virtual void extraInit();
    virtual void start();
    virtual void step(const unsigned int currentStep);
    virtual void finish();
    void cleanAfterSimulation();
    void unloadModules();

The typical order is:

1. ``initializeCC3D`` initializes the Potts object, parallel utilities,
   metadata, plugins, and steppables.
2. ``extraInit`` gives plugins and steppables a second initialization
   pass after all modules have been constructed and registered.
3. ``start`` calls ``start`` on active steppables and marks the simulator
   as stepping.
4. ``step`` runs one MCS: first Potts copy attempts, then steppables.
5. ``finish`` runs optional annealing steps, calls steppable ``finish``,
   and unloads modules.
6. ``cleanAfterSimulation`` clears simulation state that must be removed
   before modules are unloaded or another simulation is started.

Initialization
--------------

``Simulator::initializeCC3D`` is the main setup function. It performs the
following high-level sequence:

.. code-block:: cpp

    initializePottsCC3D(ps.pottsCC3DXMLElement);

    pUtils->init(potts.getCellFieldG()->getDim());
    potts.setParallelUtils(pUtils);

    pUtilsSingle->init(potts.getCellFieldG()->getDim());
    processMetadataCC3D(ps.metadataCC3DXMLElement);

    // Load and initialize plugins.
    // Load and initialize steppables.

The order matters. ``Potts3D`` must create the cell field before parallel
utilities can be initialized from the lattice dimensions. Metadata is
processed after the main parallel utility object exists because metadata
can affect parallel behavior, including modules that should run in a
single-threaded mode.

After Potts and parallel utilities are ready, the simulator loads plugins
from the parsed ``<Plugin>`` elements and steppables from the parsed
``<Steppable>`` elements. Plugins are initialized through
``pluginManager``. Steppables are initialized through ``steppableManager``
and then added to ``ClassRegistry``.

Initializing Potts3D
--------------------

``initializePottsCC3D`` reads the ``<Potts>`` section and configures the
``Potts3D`` object. The first step is to register Potts as a steerable
object:

.. code-block:: cpp

    registerSteerableObject(&potts);

Then it reads settings such as:

* ``Dimensions``
* ``Steps``
* ``Anneal``
* ``Temperature`` or ``FluctuationAmplitude``
* ``CellMotility`` or cell-type fluctuation amplitudes
* ``Flip2DimRatio``
* ``MetropolisAlgorithm``
* ``RandomNumberGenerator`` and ``RandomSeed``
* boundary conditions
* lattice and dimension type
* neighbor order or maximum flip-neighbor distance

The dimensions are mandatory. Once dimensions have been parsed, the
simulator creates the Potts cell field:

.. code-block:: cpp

    if (!(ppdCC3DPtr->dim.x != 0 || ppdCC3DPtr->dim.y != 0 || ppdCC3DPtr->dim.z != 0))
        throw CC3DException("You must set Dimensions!");

    potts.createCellField(ppdCC3DPtr->dim);

The simulator also instantiates ``BoundaryStrategy`` while processing the
Potts section. This is the object that later answers neighbor queries for
``Potts3D`` during copy attempts.

Plugins and Steppables
----------------------

CC3D separates plugins from steppables.

Plugins usually provide low-level services or Potts extensions. A plugin
may register an energy function, a lattice watcher, cell attributes, or
other callbacks with ``Potts3D``. Plugins are initialized during
``initializeCC3D``:

.. code-block:: cpp

    Plugin *plugin = pluginManager.get(pluginName, &pluginAlreadyRegisteredFlag);
    if (!pluginAlreadyRegisteredFlag) {
        plugin->init(this, ps.pluginCC3DXMLElementVector[i]);
    }

Steppables are simulation-level modules that run on an MCS schedule.
They are also initialized during ``initializeCC3D``, then stored in
``ClassRegistry``:

.. code-block:: cpp

    Steppable *steppable = steppableManager.get(
        steppableName,
        &steppableAlreadyRegisteredFlag
    );

    if (!steppableAlreadyRegisteredFlag) {
        if (ps.steppableCC3DXMLElementVector[i]->findAttribute("Frequency"))
            steppable->frequency =
                ps.steppableCC3DXMLElementVector[i]->getAttributeAsUInt("Frequency");

        steppable->init(this, ps.steppableCC3DXMLElementVector[i]);
        classRegistry->addStepper(steppableName, steppable);
    }

This is the main difference between Potts-level callbacks and
simulation-level steppables:

* Potts-level ``Stepper`` objects are called inside the Potts copy-attempt
  machinery.
* Simulation-level ``Steppable`` objects are called by ``ClassRegistry``
  after Potts finishes an MCS.

The Per-MCS Step
----------------

``Simulator::step`` implements the per-MCS execution order:

.. code-block:: cpp

    Dim3D dim = potts.getCellFieldG()->getDim();
    int flipAttempts = (int)(dim.x * dim.y * dim.z * ppdCC3DPtr->flip2DimRatio);
    int flips = potts.metropolis(flipAttempts, ppdCC3DPtr->temperature);

    currstep = currentStep;
    classRegistry->step(currentStep);

This is the most important runtime sequence in the C++ core:

1. Compute the number of Potts copy attempts from lattice volume and
   ``Flip2DimRatio``.
2. Run ``potts.metropolis`` with the configured fluctuation amplitude.
3. Update the simulator's current step.
4. Run active steppables whose frequency divides the current MCS.

``ClassRegistry::step`` handles the steppable frequency check:

.. code-block:: cpp

    void ClassRegistry::step(const unsigned int currentStep) {
        for (it = activeSteppers.begin(); it != activeSteppers.end(); it++) {
            if ((*it)->frequency && (currentStep % (*it)->frequency) == 0)
                (*it)->step(currentStep);
        }
    }

This means a steppable with ``Frequency="10"`` runs at MCS values that
are multiples of 10.

Extra Initialization
--------------------

``Simulator::extraInit`` gives plugins and steppables a second chance to
initialize after all requested modules exist:

.. code-block:: cpp

    for (it = infos->begin(); it != infos->end(); it++)
        if (pluginManager.isLoaded((*it)->getName())) {
            pluginManager.get((*it)->getName())->extraInit(this);
        }

    classRegistry->extraInit(this);

This second pass is useful when one module depends on another module that
may not have existed during the first ``init`` call. For plugin authors,
``extraInit`` is the right place for setup that requires other plugins,
fields, or steppables to be available.

Start and Finish
----------------

``Simulator::start`` calls ``start`` on every active steppable through
``ClassRegistry`` and initializes runtime bookkeeping:

.. code-block:: cpp

    classRegistry->start();
    currstep = 0;
    simulatorIsStepping = true;

``Simulator::finish`` first sets the Potts temperature to zero and runs
any configured annealing steps. It then calls ``finish`` on active
steppables and unloads dynamically loaded modules:

.. code-block:: cpp

    ppdCC3DPtr->temperature = 0.0;

    for (unsigned int i = 1; i <= ppdCC3DPtr->anneal; i++)
        step(ppdCC3DPtr->numSteps + i);

    classRegistry->finish();
    unloadModules();

Annealing lets the model perform additional zero-temperature relaxation
steps after the normal simulation steps have completed.

Concentration Fields
--------------------

The simulator keeps a map from concentration-field names to field
pointers:

.. code-block:: cpp

    std::map<std::string, Field3D<float>*> concentrationFieldNameMap;

Modules can register fields and retrieve them by name:

.. code-block:: cpp

    void registerConcentrationField(std::string _name, Field3D<float>* _fieldPtr);
    Field3D<float>* getConcentrationFieldByName(std::string _fieldName);
    std::vector<std::string> getConcentrationFieldNameVector();

This is how plugins and steppables share concentration fields without
hard-coding ownership relationships. A PDE solver can create or register
a field, and another module can later retrieve that field by name.

Steering
--------

``Simulator`` stores steerable objects in ``steerableObjectMap``:

.. code-block:: cpp

    std::map<std::string, SteerableObject *> steerableObjectMap;

Objects such as ``Potts3D`` can be registered as steerable objects. This
allows CC3D to update selected module parameters while the simulation is
running. ``Simulator::steer`` dispatches updates to registered steerable
objects based on the steering data available to the simulator.

Parallel Utilities and Metadata
-------------------------------

The simulator owns two ``ParallelUtilsOpenMP`` objects:

* ``pUtils`` for the normal simulation parallel configuration.
* ``pUtilsSingle`` for modules that must run using a single worker.

During initialization, ``pUtils`` is attached to ``Potts3D`` by default.
Metadata can later request single-thread behavior for selected modules.
This matters for modules that are not safe or efficient to run in
parallel.

The important point for C++ developers is that parallel behavior is not
controlled only inside ``Potts3D``. The simulator initializes the
parallel utilities, processes metadata, and then Potts and other modules
use the configured utility objects.

Cleanup
-------

Cleanup is part of the simulator's responsibility because dynamically
loaded modules can own resources, register callbacks, and attach dynamic
cell attributes. The simulator provides two relevant methods:

* ``cleanAfterSimulation`` clears simulation state that must be removed
  before a new run.
* ``unloadModules`` unloads dynamically loaded plugins and steppables.

This order is important. Cell inventory and dynamic attributes need to be
cleaned while the relevant plugin code is still available. Unloading
modules too early would leave the simulator with callbacks or attributes
whose implementation code is gone.

How to Read the Core Control Flow
---------------------------------

When reading CC3D C++ code, follow this path first:

1. ``Simulator::initializeCC3D`` to see how the simulation is assembled.
2. ``Simulator::initializePottsCC3D`` to see how the ``<Potts>`` section
   becomes ``Potts3D`` state.
3. Plugin ``init`` methods to see what they register with ``Potts3D``.
4. ``Simulator::step`` to see the MCS-level schedule.
5. ``Potts3D::metropolisFast`` to see individual copy attempts.
6. ``ClassRegistry::step`` to see when simulation-level steppables run.

This path gives the most useful mental model: ``Simulator`` owns the run,
``Potts3D`` owns pixel-copy dynamics, plugins extend Potts behavior, and
``ClassRegistry`` schedules steppables after each MCS.
