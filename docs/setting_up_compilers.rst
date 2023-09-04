Setting up Windows computer for DeveloperZone compilation and developing new CC3D plugins and Steppables in C++
================================================================================================
If you want tot develop plugins and steppables under windows you will need to install free Visual Studio
2015 Community Version. The installation of this package is straightforward but you need to make sure that you are
installing C/C++ compilers when the installer gives you options to select which programming languages
you would like to have support for. The best way to download Visual Studio is to
get it directly from Microsoft website. Current link to visual studio download page is
here: https://visualstudio.microsoft.com/vs/older-downloads/ .
Just make sure you scroll down and find Visual Studio 2015. It has to be exactly this version. CompuCell3D compilation
will likely not work with other versions. Once you download and install Visual Studio 2015 you are ready to start
compiling Developer Zone projects and develop your own CompuCell3D plugins and steppables in C++.

Setting up Linux computer for DeveloperZone compilation and developing new CC3D plugins and Steppables in C++
================================================================================================

If you are using linux computer , most likely you do not need to do any compiler setup. CC3D on Linux comes prepackaged
with all compilers and it does not matter if you installed CC3D using automated installer or installed directly using
``conda install`` command.

Setting up your Mac for DeveloperZone compilation and developing new CC3D plugins and Steppables in C++
================================================================================================

Starting with CC3D 4.3.0 when you install CC3D it will come with most of the tools needed to compile C+++
plugins and steppables. The only thing that you need in addition to this is to install ``xcode-select`` package
To install this from the terminal run the following:

.. code-block:: console

    xcode-select --install

This is it and you should be ready to compile custom plugins and steppables written in C++

Setting up your Mac for CC3D compilation via conda
===================================================

Sometimes you may want to compile entire CC3D C++ code using conda build system. In general when compiling
code via conda-build system you do not need to install anything - besides manking sure that your conda-build
system works properly. To ensure that conda build system works properly from your ``base`` conda environment (
it is important that this is ``base`` environment or else things may not work properly) run

.. code-block:: console

    conda install conda-build

This will install all utilities you need to build CC3D. Tools like swig, cmake, compilers etc will be downloaded
and installed automatically just in time for compilation. We will only mention that Current version of CC3D uses
clang compilers version 12. On Linux we use gcc compilers and on Windwos Visual Studio 2015 Community Version (free)

To compile CC3D on your Mac using conda-build system follow this procedure:

1. Install ``xcode-select`` - see above
2. install miniconda3 with Python 3.7 - https://repo.anaconda.com/miniconda/Miniconda3-py37_4.11.0-MacOSX-x86_64.sh .
Once you install miniconda in the ``base`` environment of newly installed miniconda install ``conda-build`` package

.. code-block:: console

    conda install conda-build

3. Get MacOS SDK 10.10 - https://github.com/phracker/MacOSX-SDKs or directly from
https://github.com/phracker/MacOSX-SDKs/releases. Here is direct link to the actual compressed folder:
https://github.com/phracker/MacOSX-SDKs/releases/download/11.3/MacOSX10.10.sdk.tar.xz
Once you unpack move the content to ``/opt`` folder of your Mac. You need to be ``admin`` to do this.
You should be able to see the following folder ``/opt/MacOSX10.10.sdk`` after the copy is complete

4. Clone CC3D repository

.. code-block:: console

    git clone https://github.com/CompuCell3D/CompuCell3D.git

5. Go to CC3D repository's ``conda-recipes`` folder:

.. code-block:: console

    cd <CC3D repository dir>/conda=recipe


6. Start compilation by typing

.. code-block:: console

    conda build . -c conda-forge -c compucell3d

After a while you should have CC3D conda package ready





