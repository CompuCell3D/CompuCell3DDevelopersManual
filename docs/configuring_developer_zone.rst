Configuring DeveloperZone Projects for compilation
==================================================

What is DeveloperZone?
----------------------

From technical viewpoint ``DeveloperZone`` is a folder that contains source code for additional plugins and steppables
written in C++. Depending on your needs, sometimes, you want to write high-performance CC3D module that runs much faster
than equivalent Python code. Up until version 4.3.0 of CC3D developing C++ modules was a little bit involved because
it required users to install and configure appropriate compilers that will work with provided binaries, performing
CMake configuration - the challenge here was to make sure that all Cmake variables point to appropriate directories,
that Python version identified by Cmake matches the one with which CC#D was compiled etc... In practice this process was
often perceived as quite error-prone.

Starting with version 4.3.0 of CC3D we provide one-click configurator for "DeveloperZone" All that is required from
the user is one time setup of compiler (on Windows you will install Visual Studio 2015 Community Edition, and on Mac
you need to install xcode-select package - all described in sections above. On linux you will likely not need any
additional setup).

Once you set up compilers (and install binaries for CC3D) open Twedit++ and go to ``CC3D C++ -> DeveloperZone ...`` .
This will open the following dialog:

|dz_001|

Before going any further, make sure you you have a working copy of the CC3D source code. The best way is to clone CC3D
source code repository. If you have git installed on your system you are ready to go. If not you can easily do it
by running ``conda-shell`` script from CC3D installation folder. Assuming your CC3D is installed
into ``c:\CompuCell3D`` (on Windows) you would run the following:

.. code-block:: console

    cd c:\
    c:\CompuCell3D\conda-shell.bat

then :

.. code-block:: console

    conda install -c conda-forge git

At this point you should have ``git`` installed within base environment of the miniconda distribution that
is bundled with CC3D. In general to make use of any conda tools you would first need to run ``conda-shell`` each time
you open new terminal.

Let's clone CC3D source code now. In the terminal where you previously ran ``conda-shell.bat``, do the following

.. code-block:: console

    cd c:\cc3d_source
    git clone https://github.com/CompuCell3D/CompuCell3D.git .

This will clone (download) CC3D source code and place it in ``c:\cc3d_source``

Now let's make build directory. This is a directory where compilers will place temporary compilation objects:

.. code-block:: console

    cd c:\
    mkdir cc3d_source_build

.. warning::

    It is important to create a fresh (empty) build directory before you can configure DeveloperZone configuration. CC3D cannot use build directory that is non empty



.. |dz_001| image:: images/dev_zone_osx_000.png
   :width: 3.725in
   :height: 1.8in
