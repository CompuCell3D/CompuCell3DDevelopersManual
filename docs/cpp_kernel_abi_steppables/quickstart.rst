Practical Quickstart: A C++ Steppable
======================================

.. note::

   This page is written as a reusable practical lesson. It deliberately
   leaves ABI layout and binary compatibility details to
   :doc:`lifecycle_and_abi`.

A kernel-ABI steppable is a small shared library that exposes one
C-compatible entry point to CC3D. The ABI does not require the implementation
to use a C++ class. It requires a set of lifecycle callbacks and a way to keep
state between those callbacks.

Function-based steppables
-------------------------

The simplest implementation uses free functions such as ``create``,
``start``, ``step``, ``finish``, and ``destroy``. State is usually represented
by a small C++ ``struct`` allocated in ``create``:

.. code-block:: cpp

   struct FieldGrowthState {
       unsigned long long executionCount = 0;
   };

   void *create() {
       return new FieldGrowthState{};
   }

   void step(CC3DKernelContext *ctx, void *statePtr) {
       auto *state = static_cast<FieldGrowthState *>(statePtr);
       ++state->executionCount;
       // Use ctx and retain data in state between calls.
   }

   void destroy(void *statePtr) {
       delete static_cast<FieldGrowthState *>(statePtr);
   }

The ``statePtr`` argument is an opaque pointer owned by the steppable. CC3D
passes the value returned by ``create`` to the later lifecycle callbacks. This
is how a function-based steppable retains counters, handles, configuration, or
other values from one callback to the next. The complete function-based
pattern is demonstrated by ``CompiledSteppableABIFields`` and its
``FieldDrivenGrowthSteppable.cpp`` source file.

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

The simulation lifecycle
------------------------

For a class-based implementation, the normal order is:

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






