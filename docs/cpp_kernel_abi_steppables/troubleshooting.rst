Troubleshooting Compiled Steppables
===================================

This page is intended for developers diagnosing a compiled module that does
not load or does not behave as expected.

The library cannot be found
---------------------------

Check the path in ``register_compiled_steppable``. It is relative to the
active simulation project unless an absolute path is used. Confirm that the
platform-specific output exists:

* ``MySteppable.dll`` on Windows;
* ``libMySteppable.so`` on Linux;
* the appropriate shared-library suffix on macOS.

The library is found but cannot be loaded
-----------------------------------------

This usually indicates a missing dependency, architecture mismatch, or runtime
environment mismatch. Confirm that:

* the library and Python process are both 64-bit or both 32-bit;
* the compiler runtime is available;
* the library was built against the intended CC3D environment;
* dependent DLLs or shared libraries can be found by the operating system.

The entry point is missing
--------------------------

The module must export this exact function:

.. code-block:: cpp

   extern "C" CC3D_STEPPABLE_EXPORT
   const CC3DSteppableV1 *cc3d_get_steppable_v1();

Common causes are missing ``extern "C"``, a different function name, or a
library that was not rebuilt after the source changed.

The process stops during initialization
----------------------------------------

Set a breakpoint in ``create``, ``init``, and ``start``. Verify that the state
pointer is non-null and that context-dependent resources are accessed only
after the context has been attached.

Do not create shared fields or query simulator-owned services in the class
constructor unless the API explicitly guarantees that the context is already
available there.

The step callback is never reached
----------------------------------

Check the Python registration frequency and confirm that the simulation runs
long enough to reach the callback. Then set a breakpoint in the adapter and in
the class member function. If the adapter is reached but the member function
is not, inspect the cast and the state returned by ``create``.

The class crashes during cleanup
--------------------------------

Make sure the object is deleted exactly once in ``destroy``. Do not delete
simulator-owned objects or treat lightweight kernel handles as owning
resources. If the crash occurs after ``finish``, inspect the class destructor
and any cached handles it uses.

Inspecting the native module
----------------------------

On Windows, use the Visual Studio Modules window to confirm that the expected
DLL and symbols are loaded. On Linux and macOS, use the platform's shared
library inspection tools to verify the exported entry point and dependencies.

The existing manual pages :doc:`../debugging_cc3d_using_visual_studio` and
:doc:`../debugging_cc3d_using_gdb` provide debugger-specific workflows.
