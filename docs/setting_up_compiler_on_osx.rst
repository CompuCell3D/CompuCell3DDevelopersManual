OSX Compiler Setup
==================

If you are on OSX machine in order to use developer zone modules you will need to use special compiler
that behaves properly in the presence of OpenMP extensions. It happened so that for whatever reason
standard gcc/g++ compilers shipped with Apple products had annoying bug that manifested itself when you tried
spawning OpenMP threads from secondary thread. Without going too much into details, it is sufficient to say that we
had to use a special version of gcc compiler that handled properly situations described above.

Therefore in order to compile your own C++ extensions on OSX (such as those bundled in DeveloperZone) you need
to set up the same compiler on your system as we have used in our compilations. The tasks required to do so
are fairly simple but the solution itself while not super elegant, it works. The solution consists of

1) Downloading compiler ``.tar.gz`` file and unpacking it
2) If they exist, temporarily renaming ``/usr/local/Cellar`` and ``/usr/local/opt`` directories
3) Copying ``/usr/local/Cellar`` and ``/usr/local/opt`` from provided compiler ``.tar.gz`` int your local
``usr/local`` folder
4) When you are done with ``DeveloperZone`` compilation , reversing the steps and restoring original content of
``/usr/local/Cellar`` and ``/usr/local/opt``

We will walk you through all those steps in detail and show you how to compile DeveloperZone C++ extensions on OSX

The good thing is that you do not need to recompile entire CC3D but rather use our binaries. This significantly
reduces effort required to develop custom C++ modules on OSX. Let's get started:

Cloning CC3D Source code repository
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be able to build additional C++ modules for CC3D you need CC3D source code be on your machine. To do so, first create
a directory for the repository (we assume you are in your ``/Users/<your user name>`` folder):

.. code-block:: console

    mkdir CC3D_DEVELOP

    cd CC3D_DEVELOP

    git clone https://github.com/CompuCell3D/CompuCell3D.git .

|dev_zone_osx_002|

Setting up the compiler
~~~~~~~~~~~~~~~~~~~~~~~~
To set up compiler that is capable of compiling CC3D code you need to verify if ``/usr/local/Cellar`` and ``/usr/local/opt`` exist on your computer.
If they do you need to copy them to ``/usr/local/Cellar_orig`` and ``/usr/local/opt_orig`` respectively. To do so
do the following:

.. code-block:: console

    sudo mv /usr/local/Cellar /usr/local/Cellar_orig
    sudo mv /usr/local/opt /usr/local/opt_orig

See the picture below:

|dev_zone_osx_000|

Next, download ``gcc_4.8_osx_bundle.tar.gz`` from https://sourceforge.net/projects/cc3d/files/DeveloperZone_4.x.x/mac/
In my case , I downloaded it to ``/Users/m/gcc_bundle`` so if you download it to ``/Users/<your user name>/gcc_bundle``
folder you should be able to follow the rest of this chapter without much difficulty.

We go to ``/Users/m/gcc_bundle`` (``/Users/<your user name>/gcc_bundle`` on your machine) and unpack
``gcc_4.8_osx_bundle.tar.gz`` and print the content of ``gcc_bundle``:

.. code-block:: console:

    cd /Users/m/gcc_bundle

    tar -zxf gcc_4.8_osx_bundle.tar.gz

    ls

We should see there ``usr`` folder that comes from unpacking of ``gcc_4.8_osx_bundle.tar.gz``. We step into this folder:
and print its content

.. code-block:: console

    cd usr/local

    ls

We should see ``Cellar`` and ``opt`` folders. Next we copy those two local folders into machine's ``/usr/local`` folder:

.. code-block::

    sudo cp -R Cellar/ /usr/local/Cellar

    sudo cp -R opt/ /usr/local/opt

At this point you should have a functioning gcc compiler on your machine that can compile CC3D. The picture below
summarizes all the above steps. Make sure to replace ``/Users/m`` with the path to your actual user directory:

|dev_zone_osx_001|




.. |dev_zone_osx_000| image:: images/dev_zone_osx_000.png
   :width: 5.8in
   :height: 1.8in


.. |dev_zone_osx_001| image:: images/dev_zone_osx_001.png
   :width: 7.8in
   :height: 2.7in

.. |dev_zone_osx_002| image:: images/dev_zone_osx_002.png
   :width: 7.8in
   :height: 2.4in



