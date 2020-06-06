Debugging CC3D using GDB
========================

Sometimes when you execute simulation and encounter software crash it is useful to do a quick
introspection to see what went wrong. IN this section we will show you how to inspect CC3D call trace
using GDB.

.. note::

    Provided recipe works only on OSX and Linux

First it is useful to create a copy of CC3D run scripts because we will be modifying those. This way
you will have a copy to revert to after you are done with debugging.


