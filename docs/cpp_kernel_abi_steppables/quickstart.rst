Practical Quickstart: A C++ Steppable
======================================

.. note::

   This page is written as a reusable practical lesson. It deliberately
   leaves ABI (Application Binary Interface) layout and binary compatibility details to
   :doc:`lifecycle_and_abi`.

For complete kernel API examples (cheatsheet), see :doc:`kernel_api_examples`.


A kernel-ABI (kernel Application Binary Interface) steppable is a small shared library that exposes one
C-compatible entry point to CC3D. Such C++ steppables coexist with Python steppables.
One implements part of the simulation logic in C++; the other does the same in Python.
A typical use case for C++ steppables is simulation logic that would significantly slow down your
simulation if implemented in Python. When you need to iterate over individual voxels of the lattice or
of a cell, you might be better off implementing this logic in C++. The speedups over equivalent Python
can be significant. The best part of a kernel-ABI steppable is that it does not require recompiling core
CC3D code. Instead, you write a relatively small C++ module, as explained below, and CompuCell3D
automatically compiles it and runs it as part of your simulation. You will need a C++ compiler installed
on your computer. We recommend Microsoft Visual Studio on Windows, g++ on Linux, and clang on macOS.

Note that the compiler you use does not have to be the same version as the compiler used to build CC3D,
but it must support the C++17 standard (or, at the very least, C++11). Modern C++ compilers generally
support these standards.

Compiler installation
----------------------

Install a compiler appropriate for your operating system. The compiler must support C++17 (or, at a
minimum, C++11).

* **Windows:** Install the `Visual Studio Build Tools <https://visualstudio.microsoft.com/downloads/>`_.
  Select the ``Desktop development with C++`` workload. A full Visual Studio installation also works.
* **Ubuntu:** Install GCC and the standard build tools with ``sudo apt update && sudo apt install
  build-essential``.
* **Red Hat Enterprise Linux/Fedora:** Install GCC and its C++ support with ``sudo dnf install gcc-c++``
  (use ``yum`` instead of ``dnf`` on older systems).
* **macOS:** Install Apple’s command-line developer tools with ``xcode-select --install``. This provides
  the clang compiler.


The ABI does not require the implementation to use a C++ class. It requires a set of lifecycle callbacks and a way to keep
state between those callbacks.


C++ steppable project layout
----------------------------

A C++ steppable is declared by a ``Resource Type="Cpp"`` entry in the
``.cc3d`` project file. E.g.:

.. code-block:: xml

   <Resource Type="Cpp">Native_GrowthSteppable/GrowthClassSteppable.cpp</Resource>

The directory containing the declared entry source is
the target root. CC3D builds one shared library for each C++ resource.

A simple modular target can use this layout:

.. code-block:: text

   MySteppable/
       MySteppable.cc3d
       Native/
           MySteppable.cpp
           Utilities/
               Vector2D.cpp
               Vector2D.h
       Simulation/
           MySteppable.py
           MySteppable.xml

The target root must contain exactly one root-level ``.cpp`` file: the entry
source named by the C++ resource. IN the above example this is ``MySteppable.cpp``. Additional implementation files must be
placed in subdirectories. CC3D recursively discovers nested ``.cpp`` files and
compiles them into the same shared library. It also tracks nested ``.h`` and
``.hpp`` files and adds the target root and discovered source directories to
the include path.

A project may contain several independent C++ steppables. Each steppable needs
its own target root:

.. code-block:: text

   CombinedDemo/
       CombinedDemo.cc3d
       Native_GrowthSteppable/
           GrowthSteppable.cpp
           Utilities/
               Vector2D.cpp
               Vector2D.h
       Native_FieldSteppable/
           FieldSteppable.cpp
           Utilities/
               FieldUtils.cpp
               FieldUtils.h
       Simulation/
           CombinedDemo.py
           CombinedDemo.xml

The project descriptor declares one resource for each target:

.. code-block:: xml

   <Resource Type="Cpp">
       Native_GrowthSteppable/GrowthSteppable.cpp
   </Resource>
   <Resource Type="Cpp">
       Native_FieldSteppable/FieldSteppable.cpp
   </Resource>

Each resource is compiled independently and produces its own shared library.
Two C++ resources must not share a target root. A target with more than one
root-level ``.cpp`` file is rejected because CC3D cannot distinguish which
file is the entry source.

CC3D keeps generated build files below ``.cc3d-build``:

.. code-block:: text

   Native_GrowthSteppable/
       GrowthSteppable.cpp
       GrowthSteppable.dll
       .cc3d-build/
           GrowthSteppable/
               GrowthSteppable.cc3d-build.json
               GrowthSteppable.obj
               GrowthSteppable.lib
               GrowthSteppable.pdb

The runtime shared library remains beside the entry source - see how  ``GrowthSteppable.cpp`` and ``GrowthSteppable.dll``
are placed directly under ``Native_GrowthSteppable``. Metadata and compiler-generated files
are stored below ``.cc3d-build/<library-name>``. CC3D creates this directory
automatically; users should not edit its contents. Player's Clean action and
the command-line cleanup options remove generated libraries, metadata, and
the target build directory. ``.cc3d-build`` is excluded from source discovery.


The simulation lifecycle
------------------------

The life-cycle of each such steppable in CC3D is as follows. It applies to built-in steppables, Python steppables and
the newly introduced C++ ABI Steppables:

.. code-block:: text

   create -> init -> start -> step -> step -> ... -> finish -> destroy

The new C++ ABI steppables follow implement ``create``, ``init`` etc..., functions that CC3D calls when it runs steppable operations.

Function-based steppables
-------------------------

The simplest implementation uses free functions such as ``create``,
``start``, ``step``, ``finish``, and ``destroy``. State is usually represented
by a small C++ ``struct`` allocated in ``create``:

.. code-block:: cpp
   :linenos:

   #include <cc3d/kernel.h>
   
   #include <cstdio>
   #include <iostream>
   #include <ostream>
   
   namespace {
       constexpr unsigned char TUMOR = 1u;
       using namespace std;
       struct FieldGrowthState {
           uint64_t executionCount;
       };
   
       void *create() {
           return new FieldGrowthState{0u};
       }
   
       void start(CC3DKernelContext *ctx, void *statePtr) {
           using namespace std;
           auto *state = static_cast<FieldGrowthState *>(statePtr);
           state->executionCount = 0u;
   
           auto seededTumor = ctx->newCell(TUMOR);
           seededTumor.targetVolume = 25.0f;
           seededTumor.lambdaVolume = 2.0f;
           ctx->cellField.assign(1, 6, 1, 6, 0, 1, seededTumor);
   
           for (auto cell : ctx->cells) {
               if (cell.type == TUMOR) {
                   cell.targetVolume = 25.0f;
                   cell.lambdaVolume = 2.0f;
               }
           }
   
           std::printf("Compiled field growth start\n");
       }
   
       void step(CC3DKernelContext *ctx, void *statePtr) {
           auto *state = static_cast<FieldGrowthState *>(statePtr);
           auto fgf = ctx->scalarFields["FGF"];
           auto dim = ctx->cellField.dim();
   
           if (!fgf) {
               std::printf("Compiled field growth missing field FGF\n");
               return;
           }
   
           bool printedSample = false;
           unsigned long medium_pixels = 0;
           unsigned long cell_pixels = 0;
           for (int z = 0; z < dim.z; ++z) {
               for (int y = 0; y < dim.y; ++y) {
                   for (int x = 0; x < dim.x; ++x) {
                       auto pixelCell = ctx->cellField(x, y, z);
                       if (!pixelCell) {
                           ++medium_pixels;
                           continue;
                       }
                       ++cell_pixels;
                   }
               }
           }
           cerr<<"medium_pixels="<<medium_pixels<<" cell_pixels="<<cell_pixels<<" totalPixels="<<cell_pixels+medium_pixels<<endl;
       }
   
       void finish(CC3DKernelContext *ctx, void *statePtr) {
           auto *state = static_cast<FieldGrowthState *>(statePtr);
           std::printf("Compiled field growth finish mcs=%llu execution=%llu\n",
                       static_cast<unsigned long long>(ctx->mcs),
                       static_cast<unsigned long long>(state->executionCount));
       }
   
       void destroy(void *statePtr) {
           delete static_cast<FieldGrowthState *>(statePtr);
       }
   
       const CC3DSteppableV1 api = {
           CC3D_KERNEL_ABI_VERSION,
           &create,
           &start,
           &step,
           &finish,
           &destroy
       };
   }
   
   extern "C" CC3D_STEPPABLE_EXPORT const CC3DSteppableV1 *cc3d_get_steppable_v1() {
       return &api;
   }

The ``statePtr`` argument is an opaque pointer owned by the steppable. CC3D
passes the value returned by ``create`` to the later lifecycle callbacks. This
is how a function-based steppable retains counters, handles, configuration, or
other values from one callback to the next. The complete function-based
pattern is demonstrated by ``CompiledSteppableABIFields`` and its
``FieldDrivenGrowthSteppable.cpp`` source file - see ``Demos/SteppablesDemosCompiled`` folder.

The callback table and exported entry point are the same for both
function-based and class-based implementations:

.. code-block:: cpp

   const CC3DSteppableV1 api = {
       CC3D_KERNEL_ABI_VERSION,
       &create,
       &start,
       &step,
       &finish,
       &destroy
   };

   extern "C" CC3D_STEPPABLE_EXPORT
   const CC3DSteppableV1 *cc3d_get_steppable_v1() {
       return &api;
   }

The function-based approach is valid and can be useful for a small steppable.
As the amount of state and lifecycle logic grows, placing that state and logic
in a class usually makes ownership and organization clearer.

Class-based steppables
----------------------

A class-based steppable treats the class instance itself as the state object.
Its data members replace the separate state ``struct``, and its methods contain
the lifecycle logic. The ABI still calls free functions, so a small adapter
casts the opaque state pointer back to the class type:

.. code-block:: cpp

   void *create() {
       return new MinimalClassSteppable();
   }

   void step(CC3DKernelContext *ctx, void *state) {
       static_cast<MinimalClassSteppable *>(state)->step(ctx);
   }

Here ``static_cast<MinimalClassSteppable *>(state)`` means: interpret the
opaque pointer supplied by the ABI as the ``MinimalClassSteppable`` instance
created by ``create``. The adapter then invokes the class method on that
instance. The same forwarding pattern is used for ``init``, ``start``, and
``finish``, while ``destroy`` casts the pointer and calls ``delete``.

The complete class-based example is included in the documentation source tree:

.. literalinclude:: examples/MinimalClassSteppable/Native/MinimalClassSteppable.cpp
   :language: cpp
   :linenos:

The class owns its state
------------------------

Keep values that must survive from one MCS to the next as private class
members. In this example, ``stepCount_`` records how many times the compiled
steppable has run:

.. code-block:: cpp

   class MinimalClassSteppable {
   public:
       void step(CC3DKernelContext *ctx);

   private:
       unsigned long long stepCount_ = 0;
   };

The class should not assume that the kernel context exists in its constructor.
The context is supplied by CC3D through ``init``, ``start``, and ``step``.
Create simulator-owned fields and other context-dependent resources in
``init`` or ``start``.

Quick guide to C++ ABI steppable lifecycle functions
----------------------------------------------------

As we mentioned above, the normal function call order is as follows:

.. code-block:: text

   create -> init -> start -> step -> step -> ... -> finish -> destroy

``init`` is optional and runs after CC3D attaches the simulation context.
``start`` runs before the first simulation step. ``step`` runs at the
configured frequency. ``finish`` runs once when the simulation ends, and
``destroy`` releases the C++ object.

For a class-based kernel-ABI steppable, the adapter code in the native
source is required boilerplate. CC3D calls the exported C functions, while the
actual implementation lives in ``MinimalClassSteppable``:

* ``create`` allocates the C++ object and returns it as an opaque ``void *``.
* ``init``, ``start``, ``step``, and ``finish`` cast that pointer back to the
  class type and forward each callback to the matching class method.
* ``destroy`` performs the matching ``delete`` so the class instance is not
  leaked.
* The ``CC3DSteppableV1`` table declares the ABI version and connects the
  lifecycle callbacks in the order expected by the kernel.
* The exported ``cc3d_get_steppable_v1`` function gives CC3D the address of
  that table. ``extern "C"`` prevents C++ name mangling, and
  ``CC3D_STEPPABLE_EXPORT`` makes the entry point visible from the shared
  library.

This adapter has the same shape for every class-based steppable. Normally only
the class name and the forwarded methods change; the adapter remains a small,
standard piece of boilerplate around the class.
The Python registration script is small:

.. literalinclude:: examples/MinimalClassSteppable/Simulation/MinimalClassSteppable.py
   :language: python
   :linenos:

The project descriptor connects the simulation files and native source:

.. literalinclude:: examples/MinimalClassSteppable/MinimalClassSteppable.cc3d
   :language: xml
   :linenos:

The XML file is ordinary CC3D simulation configuration. The compiled
steppable does not need a special XML element because Python registers the
native library:

.. literalinclude:: examples/MinimalClassSteppable/Simulation/MinimalClassSteppable.xml
   :language: xml
   :linenos:

Practical checklist
-------------------

When creating a new class-based steppable, copy this checklist:

* Include ``<cc3d/kernel.h>``.
* Put simulation state in the class.
* Use ``init`` for context-dependent resources that must exist before
  ``start``.
* Use ``start`` for one-time simulation initialization.
* Use ``step`` for work repeated during the simulation.
* Use ``finish`` for final reporting or output.
* Allocate the class in ``create`` and delete it in ``destroy``.
* Register the same library basename in the Python script and build output.
* Export ``cc3d_get_steppable_v1`` with C linkage.

This checklist and the example code are intentionally independent of the
documentation site layout. A user manual can reuse this page and the
``examples`` directory directly.








