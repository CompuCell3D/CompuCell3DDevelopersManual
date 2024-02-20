Simulator
=========

``Simulator`` is the key C++ module that sits at the root of each simulation run by CC3D. This is essentially a single class
``Simulator`` and it is responsible for orchestrating the flow of each CC3D simulation. Simulator object creates and
manages other key objects such as ``Potts3D`` and ensures the integrity of the entire simulation.
The code for the object is stored in ``CompuCell3D\Simulator.h`` and ``CompuCell3D\Simulator.cpp``.
If there is one thing to remember about the Simulator object is that it interchangeably calls function
from Potts that implements MOnt Carlo Step (e.g. ``metropolisFast`` function) followed by call to all steppables (modules that are called after each Monte Carlo Step)

Let us look at the header file of the ``Simulator`` to examine the responsibilities that ``Simulator`` when running CC3D
simulations

.. code-block:: cpp

    namespace CompuCell3D {
        class ClassRegistry;
        class BoundaryStrategy;

        template <typename Y> class Field3DImpl;
        class Serializer;
        class PottsParseData;
        class ParallelUtilsOpenMP;

        class COMPUCELLLIB_EXPORT Simulator : public Steppable {

            ClassRegistry *classRegistry;

            Potts3D potts;

            int currstep;

            bool simulatorIsStepping;
            bool readPottsSectionFromXML;
            std::map<std::string,Field3D<float>*> concentrationFieldNameMap;
            //map of steerable objects
            std::map<std::string,SteerableObject *> steerableObjectMap;

            std::vector<Serializer*> serializerVec;
            std::string recentErrorMessage;
            bool newPlayerFlag;

            std::streambuf * cerrStreamBufOrig;
            std::streambuf * coutStreamBufOrig;
            CustomStreamBufferBase * qStreambufPtr;

            std::string basePath;
            bool restartEnabled;

        public:

            ParserStorage ps;
            PottsParseData * ppdCC3DPtr;
            PottsParseData ppd;
            PottsParseData *ppdPtr;
            ParallelUtilsOpenMP *pUtils;
            ParallelUtilsOpenMP *pUtilsSingle; // stores same information as pUtils but assumes that we use only single CPU - used in modules for which user requests single CPU runs e.g. Potts with large cells

            double simValue;

            void setOutputRedirectionTarget(ptrdiff_t  _ptr);
            ptrdiff_t getCerrStreamBufOrig();
            void restoreCerrStreamBufOrig(ptrdiff_t _ptr);

            void setRestartEnabled(bool _restartEnabled){restartEnabled=_restartEnabled;}
            bool getRestartEnabled(){return restartEnabled;}

            static PluginManager<Plugin> pluginManager;
            static PluginManager<Steppable> steppableManager;
            static BasicPluginManager<PluginBase> pluginBaseManager;
            Simulator();
            virtual ~Simulator();
            //     PluginManager::plugins_t & getPluginMap(){return pluginManager.getPluginMap();}

            //Error handling functions
            std::string getRecentErrorMessage(){return recentErrorMessage;}
            void setNewPlayerFlag(bool _flag){newPlayerFlag=_flag;}
            bool getNewPlayerFlag(){return newPlayerFlag;}

            std::string getBasePath(){return basePath;}
            void setBasePath(std::string _bp){basePath=_bp;}

            ParallelUtilsOpenMP * getParallelUtils(){return pUtils;}
            ParallelUtilsOpenMP * getParallelUtilsSingleThread(){return pUtilsSingle;}

            BoundaryStrategy * getBoundaryStrategy();
            void registerSteerableObject(SteerableObject *);
            void unregisterSteerableObject(const std::string & );
            SteerableObject * getSteerableObject(const std::string & _objectName);

            void setNumSteps(unsigned int _numSteps){ppdCC3DPtr->numSteps=_numSteps;}
            unsigned int getNumSteps() {return ppdCC3DPtr->numSteps;}
            int getStep() {return currstep;}
            void setStep(int currstep) { this->currstep = currstep; }
            bool isStepping(){return simulatorIsStepping;}
            double getFlip2DimRatio(){return ppdCC3DPtr->flip2DimRatio;}
            Potts3D *getPotts() {return &potts;}
            Simulator *getSimulatorPtr(){return this;}
            ClassRegistry *getClassRegistry() {return classRegistry;}

            void registerConcentrationField(std::string _name,Field3D<float>* _fieldPtr);
            std::map<std::string,Field3D<float>*> & getConcentrationFieldNameMap(){
                return concentrationFieldNameMap;
            }
            void postEvent(CC3DEvent & _ev);

            std::vector<std::string> getConcentrationFieldNameVector();
            Field3D<float>* getConcentrationFieldByName(std::string _fieldName);

            void registerSerializer(Serializer * _serializerPtr){serializerVec.push_back(_serializerPtr);}
            virtual void serialize();

            // Begin Steppable interface
            virtual void start();
            virtual void extraInit();///initialize plugins after all steppables have been initialized
            virtual void step(const unsigned int currentStep);
            virtual void finish();
            // End Steppable interface

            //these two functions are necessary to implement proper cleanup after the simulation
            //1. First it cleans cell inventory, deallocating all dynamic attributes - this has to be done before unloading modules
            //2. It unloads dynamic CC3D modules - plugins and steppables
            void cleanAfterSimulation();
            //unloads all the plugins - plugin destructors are called
            void unloadModules();

            void initializePottsCC3D(CC3DXMLElement * _xmlData);
            void processMetadataCC3D(CC3DXMLElement * _xmlData);

            void initializeCC3D();
            void setPottsParseData(PottsParseData * _ppdPtr){ppdPtr=_ppdPtr;}
            CC3DXMLElement * getCC3DModuleData(std::string _moduleType,std::string _moduleName="");
            void updateCC3DModule(CC3DXMLElement *_element);
            void steer();

        };
    };

Few things to notice:

1. All CompuCell3D classes are defined within ``CompuCell3D``  namespace:

.. code-block:: cpp

    namespace CompuCell3D {
        class ClassRegistry;
        ...
        class COMPUCELLLIB_EXPORT Simulator : public Steppable {
        ...
        };
    };

2. Most CC3D objects are dynamically loaded. To make sure an object can be dynamically loaded on Windows we need
to include ``__decl(dllimport)`` and ``__decl(dllexport)`` class decorators as introduced and required by Microsoft
Visual Studio Compilers. Therefore the C++ macro you see above  -``COMPUCELLLIB_EXPORT`` contains required decorators
on Windows and is an empty string on all other operating systems. You can find the details of the Microsoft decorators here:

- https://stackoverflow.com/questions/14980649/macro-for-dllexport-dllimport-switch

3. ``Simulator`` contains ``Potts3D`` object :

.. code-block:: cpp

    namespace CompuCell3D {
        class ClassRegistry;
        ...
        class COMPUCELLLIB_EXPORT Simulator : public Steppable {
            ClassRegistry *classRegistry;

            Potts3D potts;
        ...
        };
    };

4. ``Simulator`` has  dictionary of every concentration field used in the simulation

.. code-block:: cpp

    std::map<std::string,Field3D<float>*> concentrationFieldNameMap;

Those fields can be accessed by external code (e.g. Plugin or Steppable code) by using the following ``Simulator`` methods:

.. code-block:: cpp

		std::vector<std::string> getConcentrationFieldNameVector();
		Field3D<float>* getConcentrationFieldByName(std::string _fieldName);

where ``getConcentrationFieldNameVector()`` retrieves a vector of names of the fields used in the simulation and
``Field3D<float>* getConcentrationFieldByName(std::string _fieldName)`` returns a pointer to a field

5. Functions/class members related to streams *e.g.* ``std::streambuf * cerrStreamBufOrig;`` are related to redirecting
output to either console or to a GUI. We will not discuss them here

6. Core simulator functionality, as far as the flow of the simulation is concerned, is implemented in the following
functions:

.. code-block:: cpp

    void initializeCC3D();
    virtual void start();
    virtual void extraInit();///initialize plugins after all steppables have been initialized
    virtual void step(const unsigned int currentStep);
    virtual void finish();

- ``void initializeCC3D()`` initializes ``Potts3D`` object based on the CC3DML content , as well as loadable modules such as Plugins and Steppables and it is the first Simulator function that is called after parsing of the CC3DML is complete

- ``void extraInit()`` is typically executed next and it calls ``extraInit`` method that is a member of every CompuCel3D plugin. Think of this function as a way of performing a second round of initialization but in the situation where all necessary objects (plugins) are instantiated and properly located inside overseeing objects (``Simulator`` / ``Potts3D``)


- ``void start()`` function calls ``start`` method for all Steppables that were requested by current simulation.

- ``void step(const unsigned int currentStep)`` method executes a single Monte Carlo Step (**MCS**) by calling ``metropolis`` method from Potts3D;

.. code-block:: cpp

    int flips = potts.metropolis(flipAttempts, ppdCC3DPtr->temperature);

and it also calls ``step`` method of every steppable requested by the simulation (including PDE solvers) by calling
``step`` method of a ``classRegistry`` member of the ``Simulator`` object. You may think about ``classRegistry`` as
of a container that stores pointers to Steppable objects. Indeed, if we looks a the
``CompuCell3D\ClassRegistry.h`` declarations we notice that ``ClassRegistry`` class is a collection of containers with
extra functionality that simplify code calls from parent objects (*e.g.* from ``Simulator``):

.. code-block:: cpp

    namespace CompuCell3D {
      class Simulator;

      class COMPUCELLLIB_EXPORT ClassRegistry : public Steppable {
        BasicClassRegistry<Steppable> steppableRegistry;

        typedef std::list<Steppable *> ActiveSteppers_t;
        ActiveSteppers_t activeSteppers;

        typedef std::map<std::string, Steppable *> ActiveSteppersMap_t;
        ActiveSteppersMap_t activeSteppersMap;

        Simulator *simulator;

        std::vector<ParseData *> steppableParseDataVector;

      public:
        ClassRegistry(Simulator *simulator);
        virtual ~ClassRegistry() {}

        Steppable *getStepper(std::string id);

         void addStepper(std::string _type, Steppable *_steppable);

        // Begin Steppable interface
        virtual void extraInit(Simulator *simulator);
        virtual void start();
        virtual void step(const unsigned int currentStep);
        virtual void finish();
        // End Steppable interface

        virtual void initModules(Simulator *_sim);
      };
    };

- Finally the ``void finish()`` method is responsible finishing the simulation. This seemingly simple task involves few critical steps: running few Monte Carlo Steps (of metropolis algorithm) with zero temperature - users specify number of those steps in the CC3DML code (in ``<Anneal>`` element), calling ``finish`` function of every steppable, unloading dynamically loaded modules (Plugins and Steppables) to ensure that subsequent simulations can run without restarting CC3D.

There are clearly more methods in the Simulator objects but the ones described perform most of the work.





