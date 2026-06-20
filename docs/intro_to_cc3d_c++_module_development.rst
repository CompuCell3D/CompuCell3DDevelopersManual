Developing CC3D C++ Modules
===========================

This section introduces the main kinds of C++ modules used to extend
CompuCell3D. The goal is not only to show where to write code, but also
to explain where that code runs in the CC3D execution loop.

Before writing a module, it is useful to keep the core control flow in
mind:

1. ``Simulator`` initializes the simulation, loads plugins and
   steppables, and owns the per-MCS schedule.
2. ``Simulator::step`` calls ``Potts3D::metropolis`` to perform Potts
   pixel-copy attempts.
3. ``Potts3D`` calls registered energy functions, lattice monitors, and
   low-level Potts steppers while processing those copy attempts.
4. After ``Potts3D`` finishes an MCS, ``Simulator`` calls active
   simulation-level steppables through ``ClassRegistry``.

This means that different extension points run at very different
frequencies. Choosing the correct module type is one of the most
important design decisions in CC3D C++ development.

Main Module Types
-----------------

Most C++ extensions fall into one of three categories.

Steppables
~~~~~~~~~~

A simulation-level ``Steppable`` is called by ``Simulator`` after each
MCS, subject to its configured frequency. PDE solvers are common examples
of steppables.

Steppables are often the easiest modules to understand because they run
at a clear point in the simulation loop. A steppable typically iterates
over cells, lattice pixels, or concentration fields and performs an
operation once per selected MCS.

Use a steppable when the operation should happen on an MCS schedule, for
example:

* updating a field once every few MCS,
* scanning all cells and changing cell properties,
* writing output,
* applying a rule that does not need to be evaluated during every Potts
  copy attempt.

Lattice Monitors
~~~~~~~~~~~~~~~~

A lattice monitor reacts when the cell lattice changes. In C++ these
objects are registered as field-change watchers on the
``WatchableField3D<CellG*>`` owned by ``Potts3D``.

The simplest example is a volume tracker. When a pixel-copy attempt is
accepted, one cell gains a pixel and another cell loses a pixel. The
volume tracker updates the corresponding ``CellG::volume`` values when
the lattice assignment changes.

Use a lattice monitor when derived cell state must stay synchronized with
accepted pixel copies, for example:

* volume,
* center of mass,
* surface or boundary information,
* neighbor relationships,
* cached plugin-specific cell attributes.

Lattice monitors are called more frequently than steppables because they
run whenever accepted lattice changes occur.

Energy Functions
~~~~~~~~~~~~~~~~

An ``EnergyFunction`` contributes to the energy change for a proposed
Potts pixel copy. During a copy attempt, ``Potts3D`` asks
``EnergyFunctionCalculator`` to call every registered energy function and
sum the results.

Energy functions are the most performance-sensitive modules because they
are evaluated for copy attempts inside ``Potts3D::metropolis``. A large
simulation may call an energy function many times per MCS.

Use an energy function when the rule affects whether a pixel copy should
be accepted. Examples include contact energy, volume constraints,
chemotaxis-like terms, and other terms that contribute to the effective
Potts energy.

Call Frequency and Performance
------------------------------

The three module types are usually called in this order of increasing
frequency:

1. Steppables: called on an MCS schedule.
2. Lattice monitors: called when accepted lattice changes occur.
3. Energy functions: called during Potts copy attempts.

This ordering has practical consequences. Code that is acceptable in a
steppable may be too slow in an energy function. When writing energy
functions, avoid unnecessary allocation, expensive searches, repeated XML
lookups, and repeated full-lattice scans. Precompute and cache what you
can during initialization or in lattice monitors.

Recommended Reading Path
------------------------

If you are new to the C++ core, read these sections before writing a
module:

* :doc:`prerequisites_cpp`
* :doc:`simulator`
* :doc:`potts`

The ``Simulator`` section explains the simulation lifecycle. The
``Potts3D`` section explains the lattice, pixel-copy attempts, energy
functions, lattice monitors, cell inventory, and dynamic cell attributes.
Those concepts are the foundation for the module tutorials that follow.

Getting the Source Code
-----------------------

To begin C++ module development, install Git and clone the CompuCell3D
source repository. On Linux and macOS, a common layout is to keep CC3D
repositories under ``~/src-cc3d``:

.. code-block:: console

    mkdir -p ~/src-cc3d
    cd ~/src-cc3d
    git clone https://github.com/CompuCell3D/CompuCell3D.git

After cloning, the source tree should be available at:

.. code-block:: console

    ~/src-cc3d/CompuCell3D

Windows users can use the same repository URL and choose an equivalent
source directory, for example ``%USERPROFILE%\src-cc3d``.

Building Before Developing
--------------------------

Because CC3D C++ modules are compiled extensions, build CompuCell3D from
source before developing new modules. Use the platform-specific build
instructions:

* Windows: :doc:`building_core_cc3d_cpp_code_windows`
* macOS: :doc:`building_core_cc3d_cpp_code_mac`
* Linux: :doc:`building_core_cc3d_cpp_code_linux`

After the core build works, you can either modify core modules directly
or use DeveloperZone for extension modules. DeveloperZone is usually the
better starting point for new plugins and steppables because it keeps
extension code separate from the core source tree:

* :doc:`configuring_developer_zone`
* :doc:`building_growth_steppable_in_main_code`
* :doc:`developing_simple_volume_tracker_plugin`

What to Decide Before Writing Code
----------------------------------

Before creating a new C++ module, answer these questions:

* Does the code need to run once per MCS, only at a configured frequency,
  or during every copy attempt?
* Does it change whether a pixel copy is accepted? If yes, it probably
  belongs in an ``EnergyFunction``.
* Does it maintain cell attributes that must stay synchronized with the
  lattice? If yes, it probably needs a lattice monitor.
* Does it operate on all cells, all pixels, or fields after an MCS? If
  yes, it is usually a ``Steppable``.
* Does it need additional per-cell data? If yes, plan how that data will
  be attached to ``CellG`` through CC3D's dynamic attribute mechanisms.

These decisions determine where the module is registered, when it is
called, and how carefully it must be optimized.
