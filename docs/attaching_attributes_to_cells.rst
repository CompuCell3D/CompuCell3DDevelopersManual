Attaching Custom Attributes To Cells
====================================

Cells in CompuCell3D are represented by ``CellG`` class - see ``CompuCell3D/core/CompuCell3D/Potts3D/Cell.h``

.. code-block:: c++

    #ifndef CELL_H
    #define CELL_H


    #ifndef PyObject_HEAD
    struct _object; //forward declare
    typedef _object PyObject; //type redefinition
    #endif

    class BasicClassGroup;

    namespace CompuCell3D {

      /**
       * A Potts3D cell.
       */

       class CellG{
       public:
          typedef unsigned char CellType_t;
          CellG():
            volume(0),
            targetVolume(0.0),
            lambdaVolume(0.0),
            surface(0),
            targetSurface(0.0),
            lambdaSurface(0.0),
            clusterSurface(0.0),
            targetClusterSurface(0.0),
            lambdaClusterSurface(0.0),
            type(0),
            xCM(0),yCM(0),zCM(0),
            xCOM(0),yCOM(0),zCOM(0),
            xCOMPrev(0),yCOMPrev(0),zCOMPrev(0),
            iXX(0), iXY(0), iXZ(0), iYY(0), iYZ(0), iZZ(0),
            lX(0.0),
            lY(0.0),
            lZ(0.0),
            lambdaVecX(0.0),
            lambdaVecY(0.0),
            lambdaVecZ(0.0),
            flag(0),
            id(0),
            clusterId(0),
            fluctAmpl(-1.0),
            lambdaMotility(0.0),
            biasVecX(0.0),
            biasVecY(0.0),
            biasVecZ(0.0),
            connectivityOn(false),
            extraAttribPtr(0),
            pyAttrib(0)


          {}
          long volume;
          float targetVolume;
          float lambdaVolume;
          double surface;
          float targetSurface;
          float angle;
          float lambdaSurface;
          double clusterSurface;
          float targetClusterSurface;
          float lambdaClusterSurface;
          unsigned char type;
          unsigned char subtype;
          double xCM,yCM,zCM; // numerator of center of mass expression (components)
          double xCOM,yCOM,zCOM; // numerator of center of mass expression (components)
          double xCOMPrev,yCOMPrev,zCOMPrev; // previous center of mass
          double iXX, iXY, iXZ, iYY, iYZ, iZZ; // tensor of inertia components
          float lX,lY,lZ; //orientation vector components - set by MomentsOfInertia Plugin - read only
          float ecc; // cell eccentricity
          float lambdaVecX,lambdaVecY,lambdaVecZ; // external potential lambda vector components
          unsigned char flag;
          float averageConcentration;
          long id;
          long clusterId;
          double fluctAmpl;
          double lambdaMotility;
          double biasVecX;
          double biasVecY;
          double biasVecZ;
          bool connectivityOn;
          BasicClassGroup *extraAttribPtr;

          PyObject *pyAttrib;
       };


      class Cell {
      };

      class CellPtr{
       public:
       Cell * cellPtr;
      };
    };
    #endif

As you can see CellG has a number of "standard" attributes. But very often you would like to add new attributes. For
example you would like to keep last 50 center of mass positions of each cell to be able to plot recent cell trajectory.
How would you do this? A simple approach would be to attach *e.g.* ``std::queue`` to the ``CellG`` class. This is a
valid approach but it has one major disadvantage. It will require you to recompile almost entire C++ code because
``CellG`` class is a core class that is used by virtually every single CompuCell3D module. Also, if you would like to
share the code with your colleague he woudl also need to recompile his or her copy of CC3D. Hence while this simple
approach would certainly work it it is not the most convenient way of adding attributes.
What about Python then? Yes, adding new attribute in Python is very simple:

.. code-block:: python

    cell.dict['cell_x_positions'] = [0.0]*50
    cell.dict['cell_y_positions'] = [0.0]*50
    cell.dict['cell_z_positions'] = [0.0]*50

Here, I added 3 attributes each one representing last 50 positions x, y, or z coordinates of center of mass. I initialized
them to be 0.0 hence the code ``[0.0]*50``. In Python when you multiply list by an integer it will return a list that
is contains multiple copies of the list you originally multiplied (in our case we will get a list with 50 zeros).

Python approach would certainly work, but what if for efficiency reasons you want to stay in C++ world. There is a
solution for this that scales nicely i.e. it does not require recomopilation of entire code and it allows to attach
any C++ class as a cell attribute. This is what we will teachi you next.

Constructing Steppable with Custom Class Attached to Each Cell
--------------------------------------------------------------

