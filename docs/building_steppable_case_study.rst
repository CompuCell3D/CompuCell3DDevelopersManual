Building Steppable
==================

It is probably best to start discussing extension of CC3D by showing a relatively sinple example of a steppable written in C++.
In typical scenario steppables are written in Python. There are three main reasons for that **1)** No compilation is required.
**2)** The code is more compact and easier to write than C++. **3)** Python has a rich set of libraries that make scientific computation
easily accessible.
However, writing a steppable in C++ is not that much more difficult, as you will see shortly, and you are almost guaranteed
that your code will run orders of magnitude faster.
Let me rephrase this last sentence - a **typical** code written in C++ is orders of magnitude faster than equivalent code written in
pure Python. Since most of the steppable code consists of iterating over all cells and adjusting their attributes, C++ will
perform this task much faster.

Getting started
---------------

Before you start developing CC3D C++ extension modules, you need to clone CC3D repository.

.. code-block:: console

    mkdir CC3D_DEVELOP
    cd CC3D_DEVELOP
    git clone https://github.com/CompuCell3D/CompuCell3D.git .

It is optional to checkout a particular branch of CC3D, but most often you will work with ``master`` branch. If ,
however, you want to checkout a branch - you woudl type something like this:

.. code-block:: console

    git checkout 4.0.0

|git_setup|

At this point you have complete code in ``CC3D_DEVELOP`` directory. And in addition

Now we open Twedit++ - you need to have "standard" installation of CC3D on your machine available - and go to ``CC3D C++`` menu and choose ``Generate New Module...`` entry and fill out the dialog box:

|twedit_steppable_wizard|

This is exactly what we did:

#. We specify a name of a steppable as ``GrowthSteppable``
#. We specify the location of when steppable code is stored ``/Users/m/CC3D_DEVELOP/CompuCell3D/core/CompuCell3D/steppables``. Note, that we point to the cloned CC3D repository that we originally stored in ``CC3D_DEVELOP`` subdirectory. It happens that the path to this repository is ``/Users/m/CC3D_DEVELOP``.
#. We select **Steppable** radio in the  **C++ Module Type** panel. We also check ``Python Wrap`` checkbox to allow generation of Python bindings of this steppable.

When we press ``OK`` button Twedit++ will generate a complete set of template files that could be compiled as-is and the steppable will run. Obviously our goal is to modify template file to generate steppable w want. In current implementation of CC3D Twedit++ generates or modifies approximately 10 files.

|twedit_generated_steppable|

As you can see in the ``CMakeLists.txt`` file Twedit++ modified this file and added
line ``ADD_SUBDIRECTORY(GrowthSteppable)``

Now, let us focus on modifying template files and creating a steppable (``GrowthSteppable``)
we specify growth rate in the XMl and allow modification of this rate from Python.

Let's first examine the header of the ``GrowthSteppable`` class:

.. code-block:: c++

    #ifndef GROWTHSTEPPABLESTEPPABLE_H
    #define GROWTHSTEPPABLESTEPPABLE_H



    #include <CompuCell3D/CC3D.h>
    #include "GrowthSteppableDLLSpecifier.h"


    namespace CompuCell3D {

      template <class T> class Field3D;

      template <class T> class WatchableField3D;


        class Potts3D;
        class Automaton;
        class BoundaryStrategy;
        class CellInventory;
        class CellG;

      class GROWTHSTEPPABLE_EXPORT GrowthSteppable : public Steppable {


        WatchableField3D<CellG *> *cellFieldG;

        Simulator * sim;

        Potts3D *potts;

        CC3DXMLElement *xmlData;

        Automaton *automaton;

        BoundaryStrategy *boundaryStrategy;

        CellInventory * cellInventoryPtr;



        Dim3D fieldDim;

      public:

        GrowthSteppable ();

        virtual ~GrowthSteppable ();

        // SimObject interface

        virtual void init(Simulator *simulator, CC3DXMLElement *_xmlData=0);

        virtual void extraInit(Simulator *simulator);

        //steppable interface

        virtual void start();

        virtual void step(const unsigned int currentStep);

        virtual void finish() {}

        //SteerableObject interface

        virtual void update(CC3DXMLElement *_xmlData, bool _fullInitFlag=false);

        virtual std::string steerableName();

         virtual std::string toString();

      };

    };

    #endif

Each steppable defines ``virtual void start()``, ``virtual void step(const unsigned int currentStep)`` and ``virtual void finish()`` functions. They have exactly the same role
as analogous functions in Python scripting. The oly differentce is that C++ steppables will be called **before** Python steppables


Let us check the generated implementation file of the Steppable (the ``.cpp`` file):

.. code-block:: c++


    #include <CompuCell3D/CC3D.h>
    using namespace CompuCell3D;
    using namespace std;
    #include "GrowthSteppable.h"
    GrowthSteppable::GrowthSteppable() : cellFieldG(0),sim(0),potts(0),xmlData(0),boundaryStrategy(0),automaton(0),cellInventoryPtr(0){}

    GrowthSteppable::~GrowthSteppable() {

    }

    void GrowthSteppable::init(Simulator *simulator, CC3DXMLElement *_xmlData) {

      xmlData=_xmlData;

      potts = simulator->getPotts();

      cellInventoryPtr=& potts->getCellInventory();

      sim=simulator;

      cellFieldG = (WatchableField3D<CellG *> *)potts->getCellFieldG();

      fieldDim=cellFieldG->getDim();

      simulator->registerSteerableObject(this);

      update(_xmlData,true);


    }

    void GrowthSteppable::extraInit(Simulator *simulator){

        //PUT YOUR CODE HERE
    }

    void GrowthSteppable::start(){

      //PUT YOUR CODE HERE

    }

    void GrowthSteppable::step(const unsigned int currentStep){

        //REPLACE SAMPLE CODE BELOW WITH YOUR OWN

        CellInventory::cellInventoryIterator cInvItr;

        CellG * cell=0;

        cerr<<"currentStep="<<currentStep<<endl;

        for(cInvItr=cellInventoryPtr->cellInventoryBegin() ; cInvItr !=cellInventoryPtr->cellInventoryEnd() ;++cInvItr )

        {

            cell=cellInventoryPtr->getCell(cInvItr);

            cerr<<"cell.id="<<cell->id<<" vol="<<cell->volume<<endl;

        }

    }

    void GrowthSteppable::update(CC3DXMLElement *_xmlData, bool _fullInitFlag){

        //PARSE XML IN THIS FUNCTION

        //For more information on XML parser function please see CC3D code or lookup XML utils API

        automaton = potts->getAutomaton();

        ASSERT_OR_THROW("CELL TYPE PLUGIN WAS NOT PROPERLY INITIALIZED YET. MAKE SURE THIS IS THE FIRST PLUGIN THAT YOU SET", automaton)

       set<unsigned char> cellTypesSet;

        CC3DXMLElement * exampleXMLElem=_xmlData->getFirstElement("Example");

        if (exampleXMLElem){

            double param=exampleXMLElem->getDouble();

            cerr<<"param="<<param<<endl;

            if(exampleXMLElem->findAttribute("Type")){

                std::string attrib=exampleXMLElem->getAttribute("Type");

                // double attrib=exampleXMLElem->getAttributeAsDouble("Type"); //in case attribute is of type double

                cerr<<"attrib="<<attrib<<endl;

            }

        }

        //boundaryStrategy has information aobut pixel neighbors

        boundaryStrategy=BoundaryStrategy::getInstance();

    }

    std::string GrowthSteppable::toString(){

       return "GrowthSteppable";

    }

    std::string GrowthSteppable::steerableName(){

       return toString();

    }


The ``step`` function the first function we will modify. In its current implementation the
generated ``step`` function already contains helpful code. Let's take a look:

.. code-block:: c++

        void GrowthSteppable::step(const unsigned int currentStep){

        CellInventory::cellInventoryIterator cInvItr;

        CellG * cell=0;

        cerr<<"currentStep="<<currentStep<<endl;

        for(cInvItr=cellInventoryPtr->cellInventoryBegin() ; cInvItr !=cellInventoryPtr->cellInventoryEnd() ;++cInvItr )

        {

            cell = cellInventoryPtr->getCell(cInvItr);

            cerr << "cell.id=" << cell->id << " vol=" << cell->volume << endl;

        }

    }

The ``for`` loop iterates over inventory of cells and prints cell id and cell volume.
To iterate over cell inventory we are using ``cellInventoryPtr`` which is a pointer to
``CellInventory`` object. The class for this object (``CellInventory``) is defined in ``Potts3D/CellInventory.h`` and implementation is in ``Potts3D/CellInventory.cpp``. INternally we are using STL(Standard Template Library - C++) maps to keep track of cells. The statement ``cellInventoryPtr->cellInventoryBegin()`` returns an iterator to cell inventory. If you look closely at the implementation files the container we are using as a cell inventory is
``std::map<CellIdentifier,CellG *>`` and CellIdentifier contains cell id and cluster id to
 uniquely identify cells. Therefore iteration over cell inventory is simply iteration over
  STL map. If you are not familiar with concept of iterators and containers of STL we
  recommend that you look up basic C++ tutorials for example:
``https://www.tutorialspoint.com/cplusplus/cpp_stl_tutorial`` .

Let us now modify the above step function and implement first version of growth steppable:


.. code-block:: c++

        void GrowthSteppable::step(const unsigned int currentStep){

        CellInventory::cellInventoryIterator cInvItr;

        CellG * cell=0;

        float growthRate = 1.0

        for(cInvItr=cellInventoryPtr->cellInventoryBegin() ; cInvItr !=cellInventoryPtr->cellInventoryEnd() ;++cInvItr )

        {

            cell = cellInventoryPtr->getCell(cInvItr);
            cell->targetVolume += growthRate ;

        }

    }

If you are familiar with CC3D Python scripting you will quickly find analogies. The only
thing we added was the following statement ``cell->targetVolume += growthRate ;``

When we compile and run this example the cells' target volume will increase by amount hardcoded in the ``growthRate`` variable which in our case is ``1.0``.

Let's take it to the next level (slowly). Now we will write a code that increases
target volume of cells but only for the first 100 MCS and only if cell type is equal to
``1``.


.. code-block:: c++

        void GrowthSteppable::step(const unsigned int currentStep){

        if (currentStep > 100)
            return;

        CellInventory::cellInventoryIterator cInvItr;

        CellG * cell=0;

        float growthRate = 1.0



        for(cInvItr=cellInventoryPtr->cellInventoryBegin() ; cInvItr !=cellInventoryPtr->cellInventoryEnd() ;++cInvItr )

        {

            cell = cellInventoryPtr->getCell(cInvItr);
            if (cell->type == 1){
                cell->targetVolume += growthRate ;
            }

        }

    }

First thing we do in this steppable is checking if current MCS is greater than ``100`` and
if so we return. Inside the loop we added ``if (cell->type == 1)`` check that allows increase of target volume only if cell is of type ``1``. Small digression here. If you
 want to print cell type to the screen you need to use the following syntax:

 .. code-block:: c++:

    cerr << "cell type=" << (int)cell->type <<endl;

As you can see we are performing type cast to ``int``. This is because cell type (defined in
``Potts3D/Cell.h``) is defined as ``unsigned char``. Consequently CC3D allows only 256 cell types, which at first sight migh look limiting but in practice is more than enough.


.. |git_setup| image:: images/git_setup.png
   :width: 6.0in
   :height: 2.2in


.. |twedit_steppable_wizard| image:: images/twedit_steppable_wizard.png
   :width: 7.8in
   :height: 4.4in

.. |twedit_generated_steppable| image:: images/twedit_generated_steppable.png
   :width: 7.8in
   :height: 4.4in


