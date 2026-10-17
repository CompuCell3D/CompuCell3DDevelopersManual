Lifecycle and ABI Contract
==========================

This page documents the kernel-facing contract defined in
``CompuCell3D/core/cc3d/kernel.h``. It is intended for developers who need to
maintain or extend the ABI.

The callback table
------------------

The current table is ``CC3DSteppableV1``. Its fields are ordered as follows:

.. code-block:: cpp

   typedef struct CC3DSteppableV1 {
       uint32_t abiVersion;
       void *(*create_fn)();
       void (*start_fn)(CC3DKernelContext *ctx, void *state);
       void (*step_fn)(CC3DKernelContext *ctx, void *state);
       void (*finish_fn)(CC3DKernelContext *ctx, void *state);
       void (*destroy_fn)(void *state);
       void (*init_fn)(CC3DKernelContext *ctx, void *state);
   } CC3DSteppableV1;

The field order is part of the ABI. A module must be compiled against a
compatible header and must report the ABI version expected by the runtime.

Callback order
--------------

The runtime normally uses the following sequence:

.. code-block:: text

   load library
       |
       v
   discover cc3d_get_steppable_v1
       |
       v
   create state
       |
       v
   attach context and call init, if present
       |
       v
   call start once
       |
       v
   call step at the configured frequency
       |
       v
   call finish once
       |
       v
   destroy state

The runtime owns the timing of these callbacks. The class should not call its
own lifecycle methods directly.

The kernel context
------------------

``CC3DKernelContext`` is also an ABI structure. Existing fields and nested API
tables have fixed offsets. New services must be added according to the ABI
rules documented in ``kernel.h`` rather than by rearranging existing fields.

The context gives a compiled steppable access to services such as:

* cell types and the cell inventory;
* cell and boundary-pixel iteration;
* scalar fields and shared fields;
* cell manipulation and mitosis;
* XML steering;
* shared Python dictionaries;
* SBML, neighbor, geometry, and focal point plasticity APIs.

Use the typed C++ wrappers supplied by ``kernel.h``. Do not copy or recreate
the internal simulator objects behind those wrappers.

Optional callbacks
------------------

``init``, ``start``, ``finish``, and ``destroy`` may be null when a module does
not need them. ``step`` is the callback that performs per-MCS work. A class
based module often supplies all callbacks for clarity, even when ``start`` or
``finish`` is empty.

Versioning discipline
---------------------

When the ABI changes, update the ABI version and rebuild the runtime and
compiled modules together. A library compiled against a newer incompatible
table must not be loaded by an older runtime.

For ordinary steppable development, use the existing ABI version and avoid
depending on undocumented fields or internal C++ classes.
