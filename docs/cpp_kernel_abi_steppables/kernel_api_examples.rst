Practical Kernel API Examples
=============================

.. note::

   This page is organized as reusable task-oriented material. Each section
   can be moved to a user manual independently.

The ``CC3DKernelContext`` passed to a steppable exposes the kernel services
that are safe to use through the ABI. A class-based steppable normally keeps
handles or counters as members and obtains the context from its lifecycle
methods.

Reading the current MCS
-----------------------

The current Monte Carlo step is available as ``ctx->mcs``:

.. code-block:: cpp

   void step(CC3DKernelContext *ctx) {
       if (ctx->mcs % 10 == 0) {
           std::cout << "MCS=" << ctx->mcs << std::endl;
       }
   }

Iterating over cells
--------------------

The cell inventory can be traversed directly:

.. code-block:: cpp

   for (auto cell : ctx->cells) {
       cell.targetVolume += 0.1f;
   }

Cell handles are lightweight ABI wrapper objects. Changes made through a
handle update the corresponding simulation cell.

Creating cells and assigning pixels
------------------------------------

The kernel context can create a cell and assign a rectangular region to it:

.. code-block:: cpp

   auto cell = ctx->newCell(1);
   cell.targetVolume = 25.0f;
   cell.lambdaVolume = 2.0f;
   ctx->cellField.assign(5, 10, 5, 10, 0, 1, cell);

The cell type value must match a type declared by the simulation. For code
that should not depend on numeric type IDs, use the cell-type API to look up a
type by name.

Reading a scalar field
----------------------

Existing scalar fields can be obtained by name:

.. code-block:: cpp

   auto field = ctx->scalarFields["FGF"];
   if (field) {
       const double concentration = field(10, 20, 0);
   }

Always check the field handle before using it. A simulation may omit a field
that a compiled module expects.

Sharing fields with Python
--------------------------

The shared-field API allows C++ and Python steppables to use the same named
scalar or vector field. A C++ module can create a field during ``init``:

.. code-block:: cpp

   scalar_ = ctx->sharedScalarFields.createScalar(
       "cpp_scalar", CC3D_FIELD_FLOAT32, 0);

   vector_ = ctx->sharedVectorFields.createVector(
       "cpp_vector", CC3D_FIELD_FLOAT32);

The Python side accesses these fields through ``self.field.cpp_scalar`` and
``self.field.cpp_vector``.

Updating XML values
-------------------

XML handles are resolved by element ID. A steppable can read or update values
while the simulation is running:

.. code-block:: cpp

   auto temperature = ctx->xml.getElement("temp");
   const double value = temperature.getValueAsDouble("cdata");
   temperature.setValue("cdata", value + 1.0);

Use stable XML element IDs and keep the XML structure documented with the
simulation that uses the compiled library.

Using shared Python data
------------------------

The shared dictionary API exchanges simple values and containers with Python:

.. code-block:: cpp

   auto shared = ctx->sharedData.sharedData(&ctx->dictionaryQueries);
   shared.setDouble("cpp_to_python", static_cast<double>(ctx->mcs));

   double pythonValue = 0.0;
   if (shared.getDouble("python_to_cpp", pythonValue)) {
       shared.setDouble("cpp_seen_python_value", pythonValue);
   }

Use explicit names for values exchanged between the two languages.

Related API examples
--------------------

The source repository contains complete class-based examples for neighbors,
focal point plasticity, mitosis, SBML, shared dictionaries, and XML steering.
They follow the same class-plus-adapter structure shown in the quickstart.
