Writing C++ Steppables With the Kernel ABI
===========================================

This guide explains how to write a compiled C++ steppable that runs in a
CC3D simulation through the kernel ABI. It starts with the practical steps
needed to create, build, register, and run a steppable, then describes the
ABI contract for developers who need to understand the boundary between the
simulation and a separately compiled library.

The practical pages are intentionally self-contained. They can be copied into
a user-oriented manual without bringing along the ABI implementation details.
The developer pages explain how the practical pattern works and how to debug
it when a library does not load or behaves unexpectedly.

.. toctree::
   :maxdepth: 2

   quickstart
   kernel_api_examples
   build_and_run
   class_design
   lifecycle_and_abi
   troubleshooting
   images/README

Before You Begin
----------------

Before following this guide, make sure that:

* CC3D has been built or installed with the compiled-steppable kernel header.
* A C++17 compiler is available in the environment used to build the library.
* The Python environment used to run the simulation can load the compiled
  CC3D package.
* The reader is comfortable with basic C++ classes and Python simulation
  scripts.

The source of truth for the ABI declarations is ``CompuCell3D/core/cc3d/kernel.h``
in the CC3D source tree. The examples in this guide use ABI version 1 of the
steppable table, represented by ``CC3DSteppableV1``.

Reusable sections
-----------------

The following pages are suitable for inclusion in a user manual:

* :doc:`quickstart` explains the minimum class and project structure.
* :doc:`kernel_api_examples` shows common simulation operations.
* :doc:`build_and_run` explains how to compile and register the library.

The following pages are primarily for developers maintaining compiled
extensions:

* :doc:`class_design` explains the adapter pattern and ownership rules.
* :doc:`lifecycle_and_abi` documents the ABI table and callback ordering.
* :doc:`troubleshooting` covers loader, ABI, and debugger failures.
