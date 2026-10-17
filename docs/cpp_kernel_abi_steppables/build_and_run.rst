Build, Register, and Run the Steppable
======================================

.. note::

   This page is a practical workflow and can be reused in a user manual.
   Compiler-specific setup is intentionally kept in the existing platform
   build pages.

Project layout
--------------

A small compiled-steppable project can use this layout:

.. code-block:: text

   MySteppable/
     MySteppable.cc3d
     Native/
       MySteppable.cpp
     Simulation/
       MySteppable.py
       MySteppable.xml

The ``.cc3d`` descriptor names the XML script, Python script, and native C++
source. The Python script registers the compiled library using a path relative
to the project directory.

Build the native library
------------------------

The exact compiler command depends on the platform and the active CC3D
environment. The command must:

* compile the native ``.cpp`` file as C++17 or later;
* include the directory containing ``cc3d/kernel.h``;
* produce a shared library or DLL;
* place the output beside the native source or in the path used by the Python
  registration script.

On Windows, a representative MSVC command is:

.. code-block:: console

   cl.exe /O2 /std:c++17 /EHsc /LD \
       Native\MySteppable.cpp \
       /I<CC3D_ENV>\include \
       /Fe:Native\MySteppable.dll

On Linux, the corresponding shape is:

.. code-block:: console

   c++ -O2 -std=c++17 -fPIC -shared \
       Native/MySteppable.cpp \
       -I<CC3D_ENV>/include \
       -o Native/libMySteppable.so

The library basename registered by Python must match the produced library.
CC3D's path helper adds the platform-specific suffix when appropriate.

Register the library
--------------------

Use ``register_compiled_steppable`` in the Python simulation script:

.. code-block:: python

   from cc3d import CompuCellSetup

   CompuCellSetup.register_compiled_steppable(
       library_path="Native/MySteppable",
       frequency=1,
   )

``frequency=1`` calls ``step`` after every MCS. A larger frequency reduces the
number of calls and is useful for work that does not need to run every MCS.

Run the simulation
------------------

Run the simulation from the same environment used to compile or install the
CC3D package:

.. code-block:: console

   python -m cc3d.run_script -i path/to/MySteppable.cc3d

The project can also be opened in Player. If the library fails to load, first
check the path, filename, architecture, and runtime environment before
debugging the C++ implementation.

Build verification checklist
----------------------------

* The compiler uses the intended ``cc3d/kernel.h``.
* The ABI version in the header matches the CC3D runtime.
* The output library exists beside the path registered in Python.
* The library exports ``cc3d_get_steppable_v1``.
* The Python process can load the library and its dependent runtime DLLs or
  shared libraries.
