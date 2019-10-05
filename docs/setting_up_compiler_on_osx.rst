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

Setting up the compiler
~~~~~~~~~~~~~~~~~~~~~~

