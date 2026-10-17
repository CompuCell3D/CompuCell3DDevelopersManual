Class Design and ABI Adapters
=============================

This page explains the implementation pattern used by the practical examples.
It is intended primarily for developers maintaining compiled modules.

Why adapters are needed
-----------------------

The kernel ABI uses C-style function pointers. A non-static C++ member
function has an implicit ``this`` parameter, so it cannot be placed directly
in a ``CC3DSteppableV1`` function pointer field.

The adapter receives the opaque state pointer supplied by the ABI, casts it
back to the class, and calls the member function:

.. code-block:: cpp

   void step(CC3DKernelContext *ctx, void *state) {
       auto *steppable =
           static_cast<MySteppable *>(state);
       steppable->step(ctx);
   }

The adapter is part of the binary boundary. The simulation-facing code does
not need to know the C++ class layout.

Ownership
---------

The class is allocated in ``create`` and released in ``destroy``:

.. code-block:: cpp

   void *create() {
       return new MySteppable();
   }

   void destroy(void *state) {
       delete static_cast<MySteppable *>(state);
   }

The class destructor must be safe after ``finish`` has run. Handles that refer
to simulator-owned resources should be treated as non-owning wrappers unless
the kernel API explicitly documents ownership.

Recommended responsibilities
----------------------------

Keep the class responsible for simulation behavior and state:

* configuration values and counters belong in members;
* context-dependent resource lookup belongs in ``init`` or ``start``;
* repeated work belongs in ``step``;
* final reporting belongs in ``finish``.

Keep the adapters responsible for ABI translation:

* converting ``void *`` to the class type;
* connecting member methods to the callback table;
* allocating and destroying the class object.

This separation keeps the implementation testable and makes it clear which
code is coupled to the ABI.

Complete table construction
---------------------------

The callback table is usually defined in the same translation unit as the
adapters:

.. code-block:: cpp

   const CC3DSteppableV1 api = {
       CC3D_KERNEL_ABI_VERSION,
       &create,
       &start,
       &step,
       &finish,
       &destroy,
       &init,
   };

The exported entry point returns the address of this table:

.. code-block:: cpp

   extern "C" CC3D_STEPPABLE_EXPORT
   const CC3DSteppableV1 *cc3d_get_steppable_v1() {
       return &api;
   }

Do not export C++ member names as the loader entry point. The C linkage and
stable entry-point name are what allow the runtime to discover the module.
