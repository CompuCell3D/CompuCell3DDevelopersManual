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
                            if (pair_index_1 != pair_index_2) {
                                this->typePairHTSurfaceMap[pair_index_2] += unit_surface;
                            }

                        }

                    }

                }

    }

We will be iterating over lattice pixels. Every lattice pixel has neighbors of different order but 1-st order neighbors
are simply adjacent pixels. ``BoundaryStrategy`` is an object that facilitates iteration over pixel neighbors and it
also keeps track of boundary conditions, pixels, adjacent to the boundary etc. so that you can write a simpler code. All
we need to do to iterate over 1-st order pixel neighbors is to know what is the maximum number of them and this is what
we do in this line:

.. code-block:: c++

    unsigned int maxNeighborIndex = this->boundaryStrategy->getMaxNeighborIndexFromNeighborOrder(1);

We get maximum index of a 1-st order pixel (``BoundaryStrategy`` keeps them in a vector and we are getting max index of
this vector). On 2D. cartesian lattice there could be up to 4 such neighbors hence the max vector index is 3 (we start
counting from 0).

We next clear the map where we store our results because each time we call this function it wil be incrementing
the values so if we did not clear we would be starting counting from different value that zero.

At line 16 we start triple loop where we iterate over all lattice pixels. This might not be the most efficient method
but it is the simplest to code.

In lines 19 and 20 we get a cell that resides at a given pixel. If the cell pointer returned is ``NULL`` we are dealing
with ``Medium`` and cell and this is why in lines 23-25 we check if the cell is different than ``NULL``
(``if (cell)``) before accessing its type. If it is null that we do not execute line 24 and the cell type is 0 as it
should be for the Medium.

At line 27 we start iterating over neighbors of the current pixel (``Point3D pt(x, y, z)``). This is where we do
actual calculations. Code in line 28 fetches one of the neighbor of pixel ``pt(x, y, z)``. In line 30 we check
if this neighbor is a valid one (e.g. if you are at the edge of the lattice we may get pixel that is outside of the
lattice and then if ``neighbor.distance`` is zero we know we are dealing with invalid pixel), hence in the line 31 we
skip the rest of the loop. If,however, the pixel is valid then we get a cell that resides at the neighboring pixel (
line 34):

.. code-block::

    nCell = this->cellFieldG->get(neighbor.pt);

In lines 35-38 we extract cell type of neighbor cell , again we have to be mindful of ``Medium`` as we did in
lines 22-25.

Finally, lines 41-45 contain actual code that increments boundary surface between two cell types. This code runs only
if ``nCell`` and ``cell`` *i.e.* cells belonging to adjacent pixels are different cells. In this case we
compute index for type of ``nCell`` and type of ``cell`` (
    ``unsigned int pair_index_1 = typePairIndex(cell_type, n_cell_type);``)

and increment appropriate entry in the ``this->typePairHTSurfaceMap`` - lines 44-45. Notice that we also permute
cell types in call to ``typePairIndex`` - line 43. so that when we access boundary length between cell type 1 and 2
it will give us the same value as between cell types 2 and 1.

Obviously there is a lot of doublecounting








