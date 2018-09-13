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
code snippets most relevant to current discussion. You are encouraged to ):

.. code-block:: cpp

	class /*DECLSPECIFIER*/ Potts3D :public SteerableObject {
		WatchableField3D<CellG *> *cellFieldG;
		AttributeAdder * attrAdder;
		EnergyFunctionCalculator * energyCalculator;

		BasicClassGroupFactory cellFactoryGroup; 	//creates aggregate of objects associated with cell
														//DOES NOT creat group of cells (as a name might suggest)

		/// An array of energy functions to be evaluated to determine energy costs.
		std::vector<EnergyFunction *> energyFunctions;
		EnergyFunction * connectivityConstraint;

		std::map<std::string, EnergyFunction *> nameToEnergyFuctionMap;


        ...

		std::vector<BasicRandomNumberGeneratorNonStatic> randNSVec;

		TypeTransition *typeTransition; //fires up automatic tasks associated with type reassignment

		/// An array of potts steppers.  These are called after each potts step.
		std::vector<Stepper *> steppers;

		std::vector<FixedStepper *> fixedSteppers;
		/// The automaton to use.  Assuming one automaton per simulation.
		Automaton* automaton;


		DefaultAcceptanceFunction defaultAcceptanceFunction;
		FirstOrderExpansionAcceptanceFunction   firstOrderExpansionAcceptanceFunction;

		/// Used to determine the probablity that a pixel flip should be taken.
		AcceptanceFunction *acceptanceFunction;

		//ExpressionEvaluatorDepot acceptanceEed;
		CustomAcceptanceFunction customAcceptanceFunction;
		bool customAcceptanceExpressionDefined;



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

		/**
		 * Create the cell field.
		 *
		 * @param dim The field dimensions.
		 */
		void createCellField(const Dim3D dim);
		void resizeCellField(const Dim3D dim, Dim3D shiftVec = Dim3D());
		//void resizeCellField(const std::vector<int> & dim, const std::vector<int> & shiftVec);

		double getTemperature() const { return temperature; }
		unsigned int getCurrentAttempt() { return currentAttempt; }
		unsigned int getNumberOfAttempts() { return numberOfAttempts; }
		unsigned int getNumberOfAttemptedEnergyCalculations() { return attemptedEC; }
		unsigned int getNumberOfAcceptedSpinFlips() { return flips; }
		void registerConnectivityConstraint(EnergyFunction * _connectivityConstraint);
		EnergyFunction * getConnectivityConstraint();

		bool checkIfFrozen(unsigned char _type);

		//std::set<Point3D> * getBoundaryPixelSetPtr() { return &boundaryPixelSet; }
		std::unordered_set<Point3D, Point3DHasher, Point3DComparator> * getBoundaryPixelSetPtr() { return &boundaryPixelSet; }

		std::unordered_set<Point3D, Point3DHasher, Point3DComparator> *  getJustInsertedBoundaryPixelSetPtr() {
			return &justInsertedBoundaryPixelSet;

		}

		std::unordered_set<Point3D, Point3DHasher, Point3DComparator> * getJustDeletedBoundaryPixelSetPtr() {
			return &justDeletedBoundaryPixelSet;
		}

		//std::set<Point3D> *  getJustInsertedBoundaryPixelSetPtr() {
		//	return &justInsertedBoundaryPixelSet;

		//}

		//std::set<Point3D> * getJustDeletedBoundaryPixelSetPtr() {
		//	return &justDeletedBoundaryPixelSet;
		//}

		std::vector<Point3D> * getBoundaryPixelVectorPtr() {
			return &boundaryPixelVector;
		}

        ...

		void initializeCellTypeMotility(std::vector<CellTypeMotilityData> & _cellTypeMotilityVector);
		void setCellTypeMotilityVec(std::vector<float> & _cellTypeMotilityVec);
		const std::vector<float> & getCellTypeMotilityVec() const { return cellTypeMotilityVec; }

		void setDebugOutputFrequency(unsigned int _freq) { debugOutputFrequency = _freq; }
		void setSimulator(Simulator *_sim) { sim = _sim; }

        ...

		Unit getMassUnit() { return massUnit; }
		Unit getLengthUnit() { return lengthUnit; }
		Unit getTimeUnit() { return timeUnit; }
		Unit getEnergyUnit() { return energyUnit; }
		bool getDisplayUnitsFlag() { return displayUnitsFlag; }

		void updateUnits(CC3DXMLElement *);

		Point3D getFlipNeighbor();

		void setBoundaryXName(std::string const & _xName) { boundary_x = _xName; }
		void setBoundaryYName(std::string const & _yName) { boundary_y = _yName; }
		void setBoundaryZName(std::string const & _zName) { boundary_z = _zName; }
		std::string const & getBoundaryXName() const { return boundary_x; }
		std::string const & getBoundaryYName() const { return boundary_y; }
		std::string const & getBoundaryZName() const { return boundary_z; }

		void setMinCoordinates(Point3D _minCoord) { minCoordinates = _minCoord; }
		void setMaxCoordinates(Point3D _maxCoord) { maxCoordinates = _maxCoord; }
		Point3D getMinCoordinates() const { return minCoordinates; }
		Point3D getMaxCoordinates() const { return maxCoordinates; }
		TypeTransition *getTypeTransition() { return typeTransition; }

		virtual void createEnergyFunction(std::string _energyFunctionType);
		EnergyFunctionCalculator * getEnergyFunctionCalculator() { return energyCalculator; }

		CellInventory &getCellInventory() { return cellInventory; }

		void clean_cell_field(bool reset_cell_inventory = true);

		virtual void registerAttributeAdder(AttributeAdder * _attrAdder);
		virtual void registerEnergyFunction(EnergyFunction *function);
		virtual void registerEnergyFunctionWithName(EnergyFunction *_function, std::string _functionName);
		virtual void unregisterEnergyFunction(std::string _functionName);

		double getNewNumber() { return energy; }
		/// Add the automaton.
		virtual void registerAutomaton(Automaton* autom);


		/// Return the automaton for this simulation.
		virtual Automaton* getAutomaton();
		void setParallelUtils(ParallelUtilsOpenMP *_pUtils) { pUtils = _pUtils; }

		virtual void registerAcceptanceFunction(AcceptanceFunction *function);

		virtual void setAcceptanceFunctionByName(std::string _acceptanceFunctionName);

		virtual AcceptanceFunction *getAcceptanceFunction() {
			return acceptanceFunction;
		}

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

		virtual double totalEnergy();

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


