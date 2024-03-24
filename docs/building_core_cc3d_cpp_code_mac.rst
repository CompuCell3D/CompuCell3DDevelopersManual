.. _My target:

Mac - OSX
=========

In order to compile entire CC3D code on Mac (not just the developer zone) you need to install OSX core developer tools:

.. code-block:: console

    xcode-select --install


For CompuCell3D development we also need miniconda installation - please follow instructions from Miniconda website:
https://docs.anaconda.com/free/miniconda/

Next let's install ``mamba`` which give you much faster package dependency resolution. Open new terminal and tun the following:

.. code-block:: console

    conda install -c conda-forge mamba


Once you have those tools you are ready to create conda environment into which we will install all the libraries and compilers that are needed for CC3D compilation. There are multiple ways to handle the installation of those prerequisites but the easiest one is to use ``environment.yaml`` files where we list all needed packages and provide this file to conda which takes care of installing them.

.. note::

    From now on I will assume that all git repositories and files have been saved to ``/Users/m/src-cc3d`` . You may want to adjust this path so that it corresponds to working folder that exists on your file system. In all commands below you would replace ``/Users/m/src-cc3d`` with the folder of your choice.



First, let's clone CompuCell3D, cc3d-player5 and cc3d-twedit5 git repositories to ``/Users/m/src-cc3d``

.. code-block:: console

    mkdir -p /Users/m/src-cc3d
    cd /Users/m/src-cc3d
    git clone https://github.com/CompuCell3D/CompuCell3D.git
    git clone https://github.com/CompuCell3D/cc3d-player5.git
    git clone https://github.com/CompuCell3D/cc3d-twedit5.git



Next, let's create file ``/Users/m/src-cc3d/environment.yaml`` with the following content:

.. code-block:: yaml

    channels:
      - conda-forge
      - compucell3d
    dependencies:
    # compile dependencies
        - cmake=3.21
        - swig>=4
        - numpy=1.24
        - clang_osx-arm64
        - clangxx_osx-arm64
        - llvm-openmp
        - python 3.10
        - numpy=1.24
        - vtk=9.2
        - eigen
        - tbb-devel=2021
        - boost=1.84
        - cmake=3.21
        - swig>=4
        - psutil
        - deprecated
        - cc3d-network-solvers>=0.3.0
    # cc3d run dependencies
        - simservice
        - notebook
        - ipywidgets
        - ipyvtklink
        - sphinx
        - graphviz
        - scipy
        - pandas
        - jinja2
    # player dependencies
        - webcolors
        - requests
        - pyqt=5
        - pyqtgraph
    # twedit dependencies
        - chardet
        - pyqtwebkit
        - qscintilla2
        - sphinx
        - pywin32 # [win]


Once we created ``environment.yaml`` let's ``cd`` to ``/Users/m/src-cc3d`` and create environment called ``cc3d_compile`` by running the following command:

.. code-block:: console

    cd /Users/m/src-cc3d
    mamba env create -f environment.yml --name cc3d_compile

The output of of the last command should look something like this

.. code-block:: console

      + yarl                                           1.9.4  py310hd125d64_0        conda-forge/osx-arm64     Cached
      + zeromq                                         4.3.5  hebf3989_1             conda-forge/osx-arm64     Cached
      + zipp                                          3.17.0  pyhd8ed1ab_0           conda-forge/noarch        Cached
      + zlib                                          1.2.13  h53f4e23_5             conda-forge/osx-arm64     Cached
      + zstd                                           1.5.5  h4f39d0f_0             conda-forge/osx-arm64     Cached

      Summary:

      Install: 337 packages

      Total download: 0 B



    Downloading and Extracting Packages

    Preparing transaction: done
    Verifying transaction: done
    Executing transaction: \
    /
    done
    #
    # To activate this environment, use
    #
    #     $ conda activate cc3d_compile
    #
    # To deactivate an active environment, use
    #
    #     $ conda deactivate



After environment in installed let's activate this environment - as suggested but above printout by running:

.. code-block:: console

    conda activate cc3d_compile


At this point we are ready to configure CompuCell3D for compilation. We will be using CMake.

.. note::

    It is important to replace ``/Users/m/src-cc3d`` with the directory into which you cloned the 3 CompuCell3D repositories repository

Let's run the following command:

.. code-block:: console

    cmake -S /Users/m/src-cc3d/CompuCell3D/CompuCell3D -B /Users/m/src-cc3d/CompuCell3D_build -DPython3_EXECUTABLE=/Users/m/miniconda3_arm64/envs/cc3d_compile/bin/python -DNO_OPENCL=ON  -DBUILD_STANDALONE=ON -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/Users/m/src-cc3d/CompuCell3D_install

Let's explain command line arguments we used when calling ``cmake`` command

``-S`` - specifies location of the CompUCdl3D source code and the actual C++ code resides indeed  in ``/Users/m/src-cc3d/CompuCell3D/CompuCell3D``

``-B`` specifies the location of the temporary compilation files

``-DPython3_EXECUTABLE=`` specifies the location of the python interpreter. Notice that it points to the conda environment we creates (``/envs/cc3d_compile/bin/python``). **Important:** depending where you installed your miniconda you may need to replace ``/Users/m/miniconda3_arm64`` with the path you miniconda installation on your machine

``-DNO_OPENCL=ON `` - is a CC3D-specific setting that tells cmake to skip generating GPU diffusion solvers. Note, the support for OpenCL on OSX is/might be problematic, hence we are using morte conservative setting and skip generation of those solvers

``-DBUILD_STANDALONE=OFF`` - is a CC3D-specific setting that tells cmake to install all python packages to python interpreter directory - i.e. inside ``/Users/m/miniconda3_arm64/envs/cc3d_compile``

``-DCMAKE_INSTALL_PREFIX=`` specifies location of installed CompuCell3D binaries

``-G "Unix Makefiles"`` instructs cmake to generate unix Makefiles that we will use for compilation of CompuCell3D


After running the las t command the output should look as follows:

.. code-block:: console

    ...
    -- Found Freetype: /Users/m/miniconda3_arm64/envs/cc3d_compile/lib/libfreetype.dylib (found version "2.12.1")
    VTK_MAJOR_VERSION=9
    NUMPY_INCLUDE_DIR
    VTK_LIB_DIRS
    THIS IS cc3d_py_source_dir: /Users/m/src-cc3d/CompuCell3D/CompuCell3D/../cc3d
    USING BUNDLE
    -- Configuring done
    CMake Warning (dev) at compucell3d_cmake_macros.cmake:200 (ADD_LIBRARY):
      Policy CMP0115 is not set: Source file extensions must be explicit.  Run
      "cmake --help-policy CMP0115" for policy details.  Use the cmake_policy
      command to set the policy and suppress this warning.

      File:

        /Users/m/src-cc3d/CompuCell3D/CompuCell3D/core/CompuCell3D/steppables/PDESolvers/hpppdesolvers.h
    Call Stack (most recent call first):
      core/CompuCell3D/steppables/PDESolvers/CMakeLists.txt:187 (ADD_COMPUCELL3D_STEPPABLE)
    This warning is for project developers.  Use -Wno-dev to suppress it.

    -- Generating done
    -- Build files have been written to: /Users/m/src-cc3d/CompuCell3D_build
    (cc3d_compile) m@Maciejs-MacBook-Pro src-cc3d %


At this point we are ready to compile CC3D:

.. code-block:: console

    cd /Users/m/src-cc3d/CompuCell3D_build
    make -j 8

We are changing to the "build directory" where or cmake, Makefile, and transient compilation files are stored and we are running ``make`` command with 8 parallel compilation threads to speed up the compilation process. The successful compilation printout should look something like that:

.. code-block:: console

    [ 99%] Linking CXX shared module _PlayerPython.so
    [ 99%] Built target PlayerPythonNew
    16 warnings generated.
    [100%] Linking CXX shared module _CompuCell.so
    [100%] Built target CompuCell


After the compilation is done we will call ```make install`

.. code-block:: console

    make install

The installed files will be placed in ``/Users/m/src-cc3d/CompuCell3D_install`` , exactly as we specified in the ``cmake`` command - ``-DCMAKE_INSTALL_PREFIX=/Users/m/src-cc3d/CompuCell3D_install``

At this point we we need to copy all ``dylib`` files from ``/Users/m/src-cc3d/CompuCell3D_install/lib`` to ``/Users/m/miniconda3_arm64/envs/cc3d_compile/lib``

.. code-block:: console

    cp /Users/m/src-cc3d/CompuCell3D_install/lib/*.dylib /Users/m/miniconda3_arm64/envs/cc3d_compile/lib


Assuming we are still in cc3d_compile conda environment (run ``conda activate cc3d_compile`` if you opened new terminal) we can run our first simulation using newly compiled CompuCell3D. We will run it without the player first and next we will show you how to get player and twedit++ working.

.. code-block::

    python -m cc3d.run_script -i /Users/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/cellsort_2D.cc3d

.. note::

    First time you execute run command on OSX it takes a while to load all the libraries. Subsequent runs start much faster

The output of the run should look something like this (remember to adjust all paths that start with ``/Users/m/src-cc3d`` to you file system folders):

.. code-block:: console

    (cc3d_compile) m@Maciejs-MacBook-Pro CompuCell3D_build % python -m cc3d.run_script -i /Users/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/cellsort_2D.cc3d
    #################################################
    # CompuCell3D Version: 4.5.0 Revision: 2
     Commit Label: f8ddda9
    #################################################
    <cc3d.core.CC3DSimulationDataHandler.CC3DSimulationData object at 0x12de43a00>
    Random number generator: MersenneTwister
    WILL RUN SIMULATION FROM BEGINNING
    CALLING FINISH


    ------------------PERFORMANCE REPORT:----------------------
    -----------------------------------------------------------
    TOTAL RUNTIME 9 s : 639 ms = 9.639 s
    -----------------------------------------------------------
    -----------------------------------------------------------
    PYTHON STEPPABLE RUNTIMES
                cellsort_2DSteppable:        0.01 ( 0.1%)
    -----------------------------------------------------------
                Total Steppable Time:        0.01 ( 0.1%)
        Compiled Code (C++) Run Time:        9.54 (99.0%)
                          Other Time:        0.08 ( 0.9%)
    -----------------------------------------------------------

