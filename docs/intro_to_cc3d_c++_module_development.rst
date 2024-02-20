Developing CC3D C++ Modules
===========================

In this and following sections we will teach you how to develop core CC3D modules. In a typical scenario modelers might be interested in developing 3 types of modules:

1. Steppables - those are the modules that Simulator calls after each MCS. Examples include e.g. PDE solvers. THose modules are the most intuitive to write because typically you either visit all cells in the simulation or all lattice pixels and perform required operations.
2. Lattice Monitors - this are the modules that are called each time CC3D functions e.g. ``metropolisFast``  makes a change to the cell lattice.  The simples example would be volume tracker  - a module responsible for incrementing the volume of the cell that extends its set of pixels due to pixel copy and decrementing the volume of th cell that loses single pixe due to pixel copy.
3. Energy Function - this module is called every pixel copy attempt because Potts algorithm calls this module to compute its contribution to the overall change of energy due to proposed pixel copy.

The above modules are ranked in increasing c all frequency with energy functions being called the most frequently followed by lattice monitors and steppables.

To begin developing CC3D modules make sure you have installation of CC3D on your computer. Next you need to install ``git`` repository management software and after this you would clone a CC3D repository into your local hard drive:

THe following commands work on Unix systems but you can easily modify them to work on Windows as well.

.. code-block:: bash

    mkdir -p ~/src-cc3d/
    cd ~/src-cc3d/
    git clone https://github.com/CompuCell3D/CompuCell3D.git

Here we created ``src-cc3d`` folder in the home directory and cloned CC3D repository there. You should see ``~/src-cc3d/CompuCell3D`` on your hard-drive.



