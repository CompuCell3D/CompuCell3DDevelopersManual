Practical Quickstart: A Class-Based C++ Steppable
==================================================

.. note::

   This page is written as a reusable practical lesson. It deliberately
   leaves ABI layout and binary compatibility details to
   :doc:`lifecycle_and_abi`.

A kernel-ABI steppable is a small shared library that exposes one C-compatible
entry point to CC3D. The implementation itself can use an ordinary C++ class.
The class stores simulation state and implements the lifecycle methods that
the simulation calls.

The smallest useful pattern has four parts:

#. A class containing state and lifecycle methods.
#. Adapter functions that forward the ABI callbacks to the class.
#. A ``CC3DSteppableV1`` table describing those callbacks.
#. An exported ``cc3d_get_steppable_v1`` function returning that table.

The complete example is included in the documentation source tree:

.. literalinclude:: examples/MinimalClassSteppable/Native/MinimalClassSteppable.cpp
   :language: cpp
   :linenos:

The class owns its state
------------------------

Keep values that must survive from one MCS to the next as private class
members. In this example, ``stepCount_`` records how many times the compiled
steppable has run:

.. code-block:: cpp

   class MinimalClassSteppable {
   public:
       void step(CC3DKernelContext *ctx);

   private:
       unsigned long long stepCount_ = 0;
   };

The class should not assume that the kernel context exists in its constructor.
The context is supplied by CC3D through ``init``, ``start``, and ``step``.
Create simulator-owned fields and other context-dependent resources in
``init`` or ``start``.

The simulation lifecycle
------------------------

For a class-based implementation, the normal order is:

.. code-block:: text

   create -> init -> start -> step -> step -> ... -> finish -> destroy

``init`` is optional and runs after CC3D attaches the simulation context.
``start`` runs before the first simulation step. ``step`` runs at the
configured frequency. ``finish`` runs once when the simulation ends, and
``destroy`` releases the C++ object.

The Python registration script is small:

.. literalinclude:: examples/MinimalClassSteppable/Simulation/MinimalClassSteppable.py
   :language: python
   :linenos:

The project descriptor connects the simulation files and native source:

.. literalinclude:: examples/MinimalClassSteppable/MinimalClassSteppable.cc3d
   :language: xml
   :linenos:

The XML file is ordinary CC3D simulation configuration. The compiled
steppable does not need a special XML element because Python registers the
native library:

.. literalinclude:: examples/MinimalClassSteppable/Simulation/MinimalClassSteppable.xml
   :language: xml
   :linenos:

Practical checklist
-------------------

When creating a new class-based steppable, copy this checklist:

* Include ``<cc3d/kernel.h>``.
* Put simulation state in the class.
* Use ``init`` for context-dependent resources that must exist before
  ``start``.
* Use ``start`` for one-time simulation initialization.
* Use ``step`` for work repeated during the simulation.
* Use ``finish`` for final reporting or output.
* Allocate the class in ``create`` and delete it in ``destroy``.
* Register the same library basename in the Python script and build output.
* Export ``cc3d_get_steppable_v1`` with C linkage.

This checklist and the example code are intentionally independent of the
documentation site layout. A user manual can reuse this page and the
``examples`` directory directly.
