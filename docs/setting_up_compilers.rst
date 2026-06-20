Setting Up Compilers
====================

CompuCell3D C++ development requires a compiler toolchain that is
compatible with the Python, CMake, VTK, SWIG, and binary libraries used
by the active CC3D build environment. The safest rule is simple: build
DeveloperZone modules with the same compiler and conda environment used
to build the core CompuCell3D package.

This page gives a high-level overview of the compiler setup. For the
complete build commands and environment files, use the platform-specific
core build instructions:

* Windows: :doc:`building_core_cc3d_cpp_code_windows`
* macOS: :doc:`building_core_cc3d_cpp_code_mac`
* Linux: :doc:`building_core_cc3d_cpp_code_linux`

Core Build vs. DeveloperZone Build
----------------------------------

There are two common C++ development workflows:

* **Core build**: compile the main CompuCell3D source tree. Use this when
  you are changing core objects such as ``Potts3D``, ``Simulator``, or
  bundled plugins.
* **DeveloperZone build**: compile external plugins, steppables, and
  Python bindings against an existing compiled CC3D installation. Use
  this when you want extension modules without modifying the core source
  tree.

Both workflows need a compatible compiler. DeveloperZone is easier to
configure after the core build works, because the core build confirms
that the compiler, Python, CMake, VTK, SWIG, OpenMP, and library paths are
consistent.

Windows Compiler Setup
----------------------

On Windows, CC3D C++ builds use the Microsoft Visual C++ compiler from
Visual Studio. The build pages currently document workflows for Visual
Studio 2015, Visual Studio 2019, and an experimental Visual Studio 2026
setup.

Install Visual Studio with the C++ desktop development tools enabled. The
important component is the MSVC C++ toolchain, not only the IDE. During
installation, make sure the selected workload includes the MSVC C++
compiler tools and the Windows SDK.

You do not need to install CMake separately through Visual Studio. The
recommended CC3D build environments install CMake through conda, together
with Python, VTK, SWIG, and other build dependencies.

For command-line builds and DeveloperZone configuration, use the Visual
Studio developer shell and the conda environment described in the Windows
core build instructions. The detailed command sequence is maintained in
:doc:`building_core_cc3d_cpp_code_windows` and
:doc:`configuring_developer_zone`.

Linux
-----

On Linux, the recommended workflow is to install compilers through the
conda environment used for CC3D compilation. The Linux build instructions
use conda packages such as ``gcc_linux-64`` and ``gxx_linux-64`` so that
the compiler and binary dependencies come from the same ecosystem.

In practice, this means you usually do not need to install a separate
system compiler for CC3D development. Create and activate the conda
environment documented in :doc:`building_core_cc3d_cpp_code_linux`, then
run CMake, ``make``, and DeveloperZone builds from that environment.

System packages may still be needed for graphics/runtime support, OpenGL,
or GPU/OpenCL-specific workflows. The Linux core build page documents
those cases separately.

macOS
-----

On macOS, install Apple's command-line developer tools first:

.. code-block:: console

    xcode-select --install

The CC3D conda build environment provides the compiler packages and most
build dependencies, for example ``clang_osx-arm64``, ``clangxx_osx-arm64``,
``llvm-openmp``, CMake, SWIG, VTK, and Python. After creating and
activating the compile environment documented in
:doc:`building_core_cc3d_cpp_code_mac`, run CMake and DeveloperZone builds
from that environment.

The exact environment name differs across older instructions. Use the
environment name that you created while following the macOS core build
page.

Conda Environment Consistency
-----------------------------

Most build problems come from mixing tools from different environments.
Avoid these combinations:

* CMake from one conda environment and Python from another.
* Twedit++ launched from a desktop shortcut while DeveloperZone expects a
  compiler environment from the terminal.
* A system compiler that does not match the libraries installed in the
  active conda environment.
* Visual Studio tools for one architecture while building a different
  architecture.

Before configuring or building C++ code, check the active environment (usually ```cc3d-compile``` as explained in e.g. :doc:`building_core_cc3d_cpp_code_windows`,  :doc:`building_core_cc3d_cpp_code_mac` or :doc:`building_core_cc3d_cpp_code_linux`):

.. code-block:: console

    conda activate cc3d-compile

followed by:

.. code-block:: console

    conda info --envs
    python -c "import sys; print(sys.executable)"
    cmake --version

On Linux and macOS, you can also check which compiler is being found:

.. code-block:: console

    which c++
    c++ --version

On Windows, verify that the Visual Studio developer shell is active by
checking that ``cl`` is available:

.. code-block:: bat

    where cl
    cl

DeveloperZone Recommendation
----------------------------

For DeveloperZone work, complete these steps in order:

1. Build core CompuCell3D successfully for your platform.
2. Activate the same conda environment used for that build.
3. On Windows, activate the matching Visual Studio developer shell.
4. Launch Twedit++ from that environment.
5. Configure DeveloperZone with the CompuCell3D repository root and a new
   empty build directory.

The detailed DeveloperZone workflow is covered in
:doc:`configuring_developer_zone`.

