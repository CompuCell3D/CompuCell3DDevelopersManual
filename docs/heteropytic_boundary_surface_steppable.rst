Computing Heterotypic Boundary Length
======================================

Heterotypic boundary surface (or length in 2D) is total surface of contact between all cells of two types. For example when you have 2
cells of type `1` and 100 cells of type `2` the heterotypic surface between thw two will be a sum of all contact
surfaces between the two types.
In this example we are not going to show every step how we generate the steppable using Twedit. We have shown this
earlier and here we will concentrate on the actual code.

This example is a bit more advanced but we will explain clearly every line of code.

The module that we generated is called ``HeterotypicBoundaryLength``. We then click ``Configure`` and ``Generate`` in
the CMake Gui and start writing actual code. We will first implement function that walks over entire lattice and
computes heterotypic surface (or length in 2D) between all cells of different types.

All C++ files can be found in ``DeveloperZone/Demos/HeterotypicBoundarySurface`` and Python bindings are , as usual in
``DeveloperZone/pyinterface/CompuCellExtraModules/CompuCellExtraModules.i``. The simulation example that uses our newly
created module is in ``DeveloperZone/Demos/HeterotypicBoundarySurface``

Here is the header file for our new steppable:

.. code-block:: c++

    #ifndef HETEROTYPICBOUNDARYLENGTHSTEPPABLE_H
    #define HETEROTYPICBOUNDARYLENGTHSTEPPABLE_H
    #include <CompuCell3D/CC3D.h>
    #include "HeterotypicBoundaryLengthDLLSpecifier.h"

    namespace CompuCell3D {

      template <class T> class Field3D;
      template <class T> class WatchableField3D;

        class Potts3D;
        class Automaton;
        class BoundaryStrategy;
        class CellInventory;
        class CellG;

      class HETEROTYPICBOUNDARYLENGTH_EXPORT HeterotypicBoundaryLength : public Steppable {

        WatchableField3D<CellG *> *cellFieldG;

        Simulator * sim;
        Potts3D *potts;
        CC3DXMLElement *xmlData;
        Automaton *automaton;
        BoundaryStrategy *boundaryStrategy;
        CellInventory * cellInventoryPtr;
        Dim3D fieldDim;

      public:

        HeterotypicBoundaryLength ();

        virtual ~HeterotypicBoundaryLength ();

        // new methods and members

        std::map<unsigned int, double> typePairHTSurfaceMap;

        unsigned int typePairIndex(unsigned int cellType1, unsigned int cellType2);
        void calculateHeterotypicSurface();
        double getHeterotypicSurface(unsigned int cellType1, unsigned int cellType2);

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

we added few methods and one class member there:

.. code-block:: c++

    // new methods and members

    std::map<unsigned int, double> typePairHTSurfaceMap;

    unsigned int typePairIndex(unsigned int cellType1, unsigned int cellType2);
    void calculateHeterotypicSurface();
    double getHeterotypicSurface(unsigned int cellType1, unsigned int cellType2);

The typePairHTSurfaceMap is a dictionary (map) that will store heterotypic boundary surface between different cell
types. Notice that we will encode pair of cell types as a single unsigned integer (hence a key to the dictionary
is unsigned integer). To do this we will use convenience function
``unsigned int typePairIndex(unsigned int cellType1, unsigned int cellType2)`` that takes as its arguments two
unsigned integers that denote cell type 1 and cell type 2. Here is the implementation of this function:

.. code-block:: c++

    unsigned int HeterotypicBoundaryLength::typePairIndex(unsigned int cellType1, unsigned int cellType2) {
        return 256 * cellType2 + cellType1;
    }

we take advantage of the fact that the number of cell types in CC3D is limited to 256 and the index this function
returns looks analogous to the index you woudl use to access a matrix if you were to store a matrix as 1D array.

Next we have two functions ``calculateHeterotypicSurface()`` that computed actual total heterotypic surface between
all cell types and ``double getHeterotypicSurface(unsigned int cellType1, unsigned int cellType2)`` that given two types
it returns a boundary between them.

Let's start analyzing code for ``calculateHeterotypicSurface`` function:

.. code-block:: c++
    :linenos:

    void HeterotypicBoundaryLength::calculateHeterotypicSurface() {

        unsigned int maxNeighborIndex = this->boundaryStrategy->getMaxNeighborIndexFromNeighborOrder(1);
        Neighbor neighbor;

        CellG *nCell = 0;

        this->typePairHTSurfaceMap.clear();

        // note: unit surface is different on a hex lattice. if you are runnign
        // this steppable on hex lattice you need to adjust it. Remember that on hex lattice unit length and unit surface have different values
        double unit_surface = 1.0;

        cerr << "Calculating HTBL for all cell type combinations" << endl;

        for (unsigned int x = 0; x < fieldDim.x; ++x)
            for (unsigned int y = 0; y < fieldDim.y; ++y)
                for (unsigned int z = 0; z < fieldDim.z; ++z) {
                    Point3D pt(x, y, z);
                    CellG *cell = this->cellFieldG->get(pt);

                    unsigned int cell_type = 0;
                    if (cell) {
                        cell_type = (unsigned int)cell->type;
                    }

                    for (unsigned int nIdx = 0; nIdx <= maxNeighborIndex; ++nIdx) {
                        neighbor = boundaryStrategy->getNeighborDirect(const_cast<Point3D&>(pt), nIdx);
                        if (!neighbor.distance) {
                            //if distance is 0 then the neighbor returned is invalid
                            continue;
                        }

                        nCell = this->cellFieldG->get(neighbor.pt);
                        unsigned int n_cell_type = 0;
                        if (nCell) {
                            n_cell_type = (unsigned int)nCell->type;
                        }


                        if (nCell != cell) {
                            unsigned int pair_index_1 = typePairIndex(cell_type, n_cell_type);
                            unsigned int pair_index_2 = typePairIndex(n_cell_type, cell_type);
                            this->typePairHTSurfaceMap[pair_index_1] += unit_surface;
                            this->typePairHTSurfaceMap[pair_index_2] += unit_surface;

                        }

                    }

                }

    }

We will be iterating over lattice pixels. Every lattice pixel has neighbors of different order but 1-st order neighbors
are simply adjacent pixels. ``BoundaryStrategy`` is an object that facilitates iteration over pixel neighbors and it
also keeps track of boundary conditions, pixels, adjacent to the boundary etc. so that you can write a simpler code. All
we need to do to iterate over 1-st order pixel neighbors is to know what is the maximum number of them