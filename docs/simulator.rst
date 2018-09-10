Simulator
---------

`Simulator` is the key C++ module that sits at the root of each simulation run by CC3D. This is essentially a single class
`Simulator` and it is responsible for orchestrating the flow of each CC3D simulation. Simulator object creates and
manages other key objects such as `Potts3D` and ensures the integrity of the entire simulation.
The code for the object is stored in `CompuCell3D\Simulator.h` and `CompuCell3D\Simulator.cpp`

Let us look at the header file of the `Simulator` to examine the responsibilities that `Simulator` when running CC3D
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
            //2. It unloads dynamic CC3D modules - pluginsd and steppables
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

1. All CompuCell3D classes are defined within CompuCell3D namespace:

.. code-block:: cpp

    namespace CompuCell3D {
        class ClassRegistry;
        ...
        class COMPUCELLLIB_EXPORT Simulator : public Steppable {
        ...
        };
    };

2. Most CC3D objects are dynamically loaded. To make sure an object can be dynamically loaded on Windows we need
to include `__decl(dllimport)` and `__decl(dllexport)` class decorators as introduced and required by Microsoft
Visual Studio Compilers. Therefore the C++ macro you see above  -` COMPUCELLLIB_EXPORT` contains required decorators
on Windows and is an empty string on all other operating systems