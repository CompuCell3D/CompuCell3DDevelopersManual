.. _My target:

Linux - Ubuntu
==============

In order to compile entire CC3D code on Linux (not just the developer zone) you need to miniconda. Once you have this tool it will take care of installing all the dependencies you need to to compie CC3D

Follow instructions from Miniconda website:
https://docs.anaconda.com/free/miniconda/

Next let's install ``mamba`` which give you much faster package dependency resolution. Open new terminal and tun the following:

.. code-block:: console

    conda install -c conda-forge mamba


Once you have those tools you are ready to create conda environment into which we will install all the libraries and compilers that are needed for CC3D compilation. There are multiple ways to handle the installation of those prerequisites but the easiest one is to use ``environment.yaml`` files where we list all needed packages and provide this file to conda which takes care of installing them.

.. note::

    From now on I will assume that all git repositories and files have been saved to ``/home/m/src-cc3d`` . You may want to adjust this path so that it corresponds to working folder that exists on your file system. In all commands below you would replace ``/home/m/src-cc3d`` with the folder of your choice.



First, let's clone CompuCell3D, cc3d-player5 and cc3d-twedit5 git repositories to ``/home/m/src-cc3d``

.. code-block:: console

    mkdir -p /home/m/src-cc3d
    cd /home/m/src-cc3d
    git clone https://github.com/CompuCell3D/CompuCell3D.git
    git clone https://github.com/CompuCell3D/cc3d-player5.git
    git clone https://github.com/CompuCell3D/cc3d-twedit5.git



Next, let's create file ``/home/m/src-cc3d/environment.yaml`` with the following content:

.. code-block:: yaml

    channels:
      - conda-forge
      - compucell3d
    dependencies:
     # compile dependencies
      - cmake=3.21
      - swig>=4
      - numpy=1.24
      - gcc_linux-64
      - gxx_linux-64
      - python 3.10
      - vtk=9.2
      - eigen
      - tbb-devel=2021
      - boost=1.84
    # libxcrypt dependency was discovered during actual compilation - searched pkgs sub-folders for all occurrences of crypt.h
      - libxcrypt
      - psutil
      - deprecated
      - cc3d-network-solvers>=0.3.0
    # cc3d run dependencies
      - scipy
      - pandas
      - jinja2
      - deprecated
      - psutil
      - simservice
      - notebook
      - ipywidgets
      - ipyvtklink
      - sphinx
      - graphviz
    # player dependencies
      - webcolors
      - requests
      - pyqt=5
      - pyqtgraph
    # twedit dependencies
      - chardet
      - pyqtwebkit
      - qscintilla2
      - pywin32 # [win]





Once we created ``environment.yaml`` let's ``cd`` to ``/home/m/src-cc3d`` and create environment called ``cc3d_compile`` by running the following command:

.. code-block:: console

    cd /home/m/src-cc3d
    mamba env create -f environment.yaml --name cc3d_compile

The output of of the last command should look something like this

.. code-block:: console

      ...
      + xorg-xproto                                   7.0.31  h7f98852_1007          conda-forge/linux-64     Cached
      + xz                                             5.2.6  h166bdaf_0             conda-forge/linux-64     Cached
      + yaml                                           0.2.5  h7f98852_2             conda-forge/linux-64     Cached
      + yarl                                           1.9.4  py310h2372a71_0        conda-forge/linux-64     Cached
      + zeromq                                         4.3.5  h59595ed_1             conda-forge/linux-64     Cached
      + zipp                                          3.17.0  pyhd8ed1ab_0           conda-forge/noarch       Cached
      + zlib                                          1.2.13  hd590300_5             conda-forge/linux-64     Cached
      + zstd                                           1.5.5  hfc55251_0             conda-forge/linux-64     Cached

      Summary:

      Install: 375 packages

      Total download: 49MB

    ──────────────────────────────────────────────────────────────────────────────────────────────────────────────────


    libxslt                                            254.3kB @   1.5MB/s  0.2s
    ipywidgets                                         113.6kB @ 671.0kB/s  0.2s
    widgetsnbextension                                 886.4kB @   2.3MB/s  0.4s
    jupyterlab_widgets                                 187.1kB @ 479.9kB/s  0.4s
    pywin32                                              8.2kB @  14.4kB/s  0.4s
    pyqtgraph                                          695.0kB @ 983.5kB/s  0.3s
    qtwebkit                                            15.6MB @   3.9MB/s  3.8s
    python                                              31.3MB @   4.4MB/s  7.0s

    Downloading and Extracting Packages

    Preparing transaction: done
    Verifying transaction: done
    Executing transaction: /
    -
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

    It is important to replace ``/home/m/src-cc3d`` with the directory into which you cloned the three CompuCell3D repositories repository

Let's run the following command:

.. code-block:: console

    cmake -S /home/m/src-cc3d/CompuCell3D/CompuCell3D -B /home/m/src-cc3d/CompuCell3D_build -DPython3_EXECUTABLE=/home/m/miniconda3/envs/cc3d_compile/bin/python -DNO_OPENCL=ON  -DBUILD_STANDALONE=OFF -DOPENGL_gl_LIBRARY=/usr/lib/x86_64-linux-gnu/libGL.so -DOPENGL_glx_LIBRARY=/usr/lib/x86_64-linux-gnu/libGLX.so -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/home/m/src-cc3d/CompuCell3D_install

Let's explain command line arguments we used when calling ``cmake`` command

``-S`` - specifies location of the CompUCdl3D source code and the actual C++ code resides indeed  in ``/home/m/src-cc3d/CompuCell3D/CompuCell3D``

``-B`` specifies the location of the temporary compilation files

``-DPython3_EXECUTABLE=`` specifies the location of the python interpreter. Notice that it points to the conda environment we creates (``/envs/cc3d_compile/bin/python``). **Important:** depending where you installed your miniconda you may need to replace ``/home/m/miniconda3`` with the path you miniconda installation on your machine

``-DNO_OPENCL=ON `` - is a CC3D-specific setting that tells cmake to skip generating GPU diffusion solvers. Note, the support for OpenCL on OSX is/might be problematic, hence we are using morte conservative setting and skip generation of those solvers

``-DBUILD_STANDALONE=OFF`` - is a CC3D-specific setting that tells cmake to install all python packages to python interpreter directory - i.e. inside ``/home/m/miniconda3/envs/cc3d_compile``

``-DCMAKE_INSTALL_PREFIX=`` - specifies location of installed CompuCell3D binaries

``-DOPENGL_gl_LIBRARY=/usr/lib/x86_64-linux-gnu/libGL.so`` - specifies location of OpenGL libraries

``-DOPENGL_glx_LIBRARY=/usr/lib/x86_64-linux-gnu/libGLX.so`` - - specifies location of OpenGL libraries

``-G "Unix Makefiles"`` instructs cmake to generate unix Makefiles that we will use for compilation of CompuCell3D

.. note::

    The two OpenGL options (``-DOPENGL_gl_LIBRARY=/usr/lib/x86_64-linux-gnu/libGL.so`` and ``-DOPENGL_glx_LIBRARY=/usr/lib/x86_64-linux-gnu/libGLX.so`` ) work only on Ubuntu. If you are compiling CC3D on different distribution e.g RedHat you may need to adjust those. Also if you have a better solution for finding those libraries using Cmake commands please share it with us!


After running the las t command the output should look as follows:

.. code-block:: console

    ...
    -- Found X11: /home/m/miniconda3/envs/cc3d_compile/include
    -- Looking for XOpenDisplay in /home/m/miniconda3/envs/cc3d_compile/lib/libX11.so;/home/m/miniconda3/envs/cc3d_compile/lib/libXext.so
    -- Looking for XOpenDisplay in /home/m/miniconda3/envs/cc3d_compile/lib/libX11.so;/home/m/miniconda3/envs/cc3d_compile/lib/libXext.so - found
    -- Looking for gethostbyname
    -- Looking for gethostbyname - found
    -- Looking for connect
    -- Looking for connect - found
    -- Looking for remove
    -- Looking for remove - found
    -- Looking for shmat
    -- Looking for shmat - found
    -- Looking for IceConnectionNumber in ICE
    -- Looking for IceConnectionNumber in ICE - found
    -- Found EXPAT: /home/m/miniconda3/envs/cc3d_compile/lib/libexpat.so (found version "2.5.0")
    -- Found double-conversion: /home/m/miniconda3/envs/cc3d_compile/lib/libdouble-conversion.so
    -- Found LZ4: /home/m/miniconda3/envs/cc3d_compile/lib/liblz4.so (found version "1.9.4")
    -- Found LZMA: /home/m/miniconda3/envs/cc3d_compile/lib/liblzma.so (found version "5.2.6")
    -- Found JPEG: /home/m/miniconda3/envs/cc3d_compile/lib/libjpeg.so (found version "80")
    -- Found TIFF: /home/m/miniconda3/envs/cc3d_compile/lib/libtiff.so (found version "4.6.0")
    -- Found Freetype: /home/m/miniconda3/envs/cc3d_compile/lib/libfreetype.so (found version "2.12.1")
    VTK_MAJOR_VERSION=9
    NUMPY_INCLUDE_DIR
    VTK_LIB_DIRS
    THIS IS cc3d_py_source_dir: /home/m/src-cc3d/CompuCell3D/CompuCell3D/../cc3d
    USING BUNDLE
    -- Configuring done
    CMake Warning (dev) at compucell3d_cmake_macros.cmake:200 (ADD_LIBRARY):
      Policy CMP0115 is not set: Source file extensions must be explicit.  Run
      "cmake --help-policy CMP0115" for policy details.  Use the cmake_policy
      command to set the policy and suppress this warning.

      File:

        /home/m/src-cc3d/CompuCell3D/CompuCell3D/core/CompuCell3D/steppables/PDESolvers/hpppdesolvers.h
    Call Stack (most recent call first):
      core/CompuCell3D/steppables/PDESolvers/CMakeLists.txt:187 (ADD_COMPUCELL3D_STEPPABLE)
    This warning is for project developers.  Use -Wno-dev to suppress it.

    -- Generating done
    -- Build files have been written to: /home/m/src-cc3d/CompuCell3D_build


At this point we are ready to compile CC3D:

.. code-block:: console

    cd /home/m/src-cc3d/CompuCell3D_build
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

The installed files will be placed in ``/home/m/src-cc3d/CompuCell3D_install`` , exactly as we specified in the ``cmake`` command - ``-DCMAKE_INSTALL_PREFIX=/home/m/src-cc3d/CompuCell3D_install``

At this point we we need to copy all ``.so`` files from ``/home/m/src-cc3d/CompuCell3D_install/lib`` to ``/home/m/miniconda3/envs/cc3d_compile/lib``

.. code-block:: console

    cp /home/m/src-cc3d/CompuCell3D_install/lib/*.so /home/m/miniconda3/envs/cc3d_compile/lib


Assuming we are still in cc3d_compile conda environment (run ``conda activate cc3d_compile`` if you opened new terminal) we can run our first simulation using newly compiled CompuCell3D. We will run it without the player first and next we will show you how to get player and twedit++ working.

.. code-block::

    python -m cc3d.run_script -i /home/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/cellsort_2D.cc3d

.. note::

    First time you execute run command on OSX it takes a while to load all the libraries. Subsequent runs start much faster

The output of the run should look something like this (remember to adjust all paths that start with ``/home/m/src-cc3d`` to you file system folders):

.. code-block:: console

    (cc3d_compile) m@tuf:~/src-cc3d/CompuCell3D_build$ python -m cc3d.run_script -i /home/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/cellsort_2D.cc3d
    #################################################
    # CompuCell3D Version: 4.5.0 Revision: 2
     Commit Label: f8ddda9
    #################################################
    <cc3d.core.CC3DSimulationDataHandler.CC3DSimulationData object at 0x7f0cfd2316f0>
    Random number generator: MersenneTwister
    WILL RUN SIMULATION FROM BEGINNING
    CALLING FINISH


    ------------------PERFORMANCE REPORT:----------------------
    -----------------------------------------------------------
    TOTAL RUNTIME 5 s : 688 ms = 5.688 s
    -----------------------------------------------------------
    -----------------------------------------------------------
    PYTHON STEPPABLE RUNTIMES
                cellsort_2DSteppable:        0.01 ( 0.2%)
    -----------------------------------------------------------
                Total Steppable Time:        0.01 ( 0.2%)
        Compiled Code (C++) Run Time:        5.60 (98.5%)
                          Other Time:        0.08 ( 1.4%)
    -----------------------------------------------------------

Using Player
-------------

To run the above simulation using player we need to make player code available to the Python interpreter from which we are running our simulation. In my case this will boil down to either copying directory ``/home/m/src-cc3d/cc3d-player5/cc3d/player5`` inside  ``/home/m/miniconda3_arm64/envs/cc3d_compile/lib/python3.10/site-packages/cc3d/player5``

or making a softlink. I prefer the softlink  and I run:

.. code-block:: console

    ln -s /home/m/src-cc3d/cc3d-player5/cc3d/player5   /home/m/miniconda3/envs/cc3d_compile/lib/python3.10/site-packages/cc3d/player5


After this step I am ready to run previous simulation using the Player:

.. code-block::

    python -m cc3d.player5

and then we would use ``File->Open...`` menu to select our ``.cc3d`` project ``/home/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/cellsort_2D.cc3d``