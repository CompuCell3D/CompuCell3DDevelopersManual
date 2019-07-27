Building Growth Steppable In the Developer Zone folder
======================================================

Quite often you will want to build a steppable in a "non-intrusive" way *i.e.* without adding it to the main
CC3D code-base. The way to do it is to utilize functionality of ``DeveloperZone``.

This time we will use Windows system and our  CC3D git repository is cloned to ``D:\CC3D_PY3_GIT\CompuCell3D``

To add a steppable or plugin in the DeveloperZone you open up Twedit and from ``CC3D C++`` menu select
``Generate New Module...``.

|dev_zone_1|

Notice that in the ``Module Directory`` in the dialog box we put ``D:\CC3D_PY3_GIT\CompuCell3D\DeveloperZone``.
Previously we put there a path to the Steppable folder in the main CC3D Code base (give where our repository is cloned
this path would be ``D:\CC3D_PY3_GIT\CompuCell3D\core\CompuCell3D\steppables\``)

Notice that we also checked ``Python Wrap`` option to generate Python bindings. We will show you how you ca
be creative here and leverage both XML and Python as a way to pass parameters to the Steppable. As you
remember you do not have to generate Python bindings and it is perfectly OK to to stick with C++ and XML.

After we press ``OK`` button Twedit++ will generate , a template Steppable code:

|dev_zone_2|

Now we copy code from our earlier example into appropriate files - we are only showing files that we modified:
``GrowthSteppable.h`` :

.. code-block:: cpp
    :linenos:

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

        std::map<unsigned int, double> growthRateMap;

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

and ``GrowthSteppable.cpp``

.. code-block:: cpp
    :linenos:



As you can see based on the previous discussion the ``update`` function where we parse XML is designed to
handle the following syntax for the GrowthSteppable:

.. code-block:: xml

    <Steppable Type="GrowthSteppable">
        <GrowthRate CellType="1" Rate="1.3"/>
        <GrowthRate CellType="2" Rate="1.7"/>
    </Steppable>

.. |dev_zone_1| image:: images/dev_zone_1.png
   :width: 2.4in
   :height: 1.9in

.. |dev_zone_2| image:: images/dev_zone_2.png
   :width: 6.0in
   :height: 2.5in