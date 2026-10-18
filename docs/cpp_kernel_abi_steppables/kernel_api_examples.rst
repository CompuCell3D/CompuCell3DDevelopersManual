Practical Kernel API Examples
=============================

.. note::

   This page is organized as reusable task-oriented material. Each section
   can be moved to a user manual independently.

The ``CC3DKernelContext`` passed to a steppable exposes the kernel services
that are safe to use through the ABI. A class-based steppable normally keeps
handles or counters as members and obtains the context from its lifecycle
methods.

Working with cells and pixels
-----------------------------

Reading the current MCS
~~~~~~~~~~~~~~~~~~~~~~~~

The current Monte Carlo step is available as ``ctx->mcs``:

.. code-block:: cpp

   void step(CC3DKernelContext *ctx) {
       if (ctx->mcs % 10 == 0) {
           std::cout << "MCS=" << ctx->mcs << std::endl;
       }
   }

Iterating over cells
~~~~~~~~~~~~~~~~~~~~

The cell inventory can be traversed directly:

.. code-block:: cpp

   for (auto cell : ctx->cells) {
       cell.targetVolume += 0.1f;
   }

Cell handles are lightweight ABI wrapper objects. Changes made through a
handle update the corresponding simulation cell.

Reading and modifying cell properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Cell handles expose commonly used properties such as the cell ID, type,
volume, surface, center of mass, target volume, and volume or surface lambda.
For example, a steppable can increase the target volume of one cell type:

.. code-block:: cpp

   for (auto cell : ctx->cells) {
       if (cell.type == 1) {
           cell.targetVolume += 1.0f;
           cell.lambdaVolume = 2.0f;
       }
   }

The same pattern can be used to read a property, make a decision, and update
the cell. The simulation must define the type values used by the module.

Creating cells and assigning pixels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The kernel context can create a cell and assign a rectangular region to it:

.. code-block:: cpp

   auto cell = ctx->newCell(1);
   cell.targetVolume = 25.0f;
   cell.lambdaVolume = 2.0f;
   ctx->cellField.assign(5, 10, 5, 10, 0, 1, cell);

The cell type value must match a type declared by the simulation. For code
that should not depend on numeric type IDs, use the cell-type API to look up a
type by name.

Finding cells by ID and type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The cell inventory also supports lookup and type-based selection. A typical
workflow is to obtain a type ID from its name, select cells of that type, and
then find a particular cell when an ID is available:

.. code-block:: cpp

   const auto cellType = ctx->cellTypes["Cell"];
   for (auto cell : ctx->cells.byType(cellType)) {
       std::cout << "cell=" << cell.id << std::endl;
   }

   auto cell = ctx->cells.fetch_cell_by_id(12);
   if (cell) {
       cell.targetVolume = 50.0f;
   }

This is useful when a cell ID is exchanged with Python or stored in shared
simulation data.

Iterating over all lattice pixels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The cell field can be traversed using the lattice dimensions. Each location
may contain a cell or medium:

.. code-block:: cpp

   const auto dim = ctx->cellField.dim();
   for (int z = 0; z < dim.z; ++z) {
       for (int y = 0; y < dim.y; ++y) {
           for (int x = 0; x < dim.x; ++x) {
               auto cell = ctx->cellField(x, y, z);
               if (cell) {
                   // Use the cell at this pixel.
               }
           }
       }
   }

The same traversal can be combined with scalar field reads or writes.

Iterating over a cell's pixels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When the operation concerns one cell rather than the whole lattice, use its
pixel iterator:

.. code-block:: cpp

   for (auto cell : ctx->cells) {
       std::size_t pixelCount = 0;
       for (auto pixel : ctx->pixels.forCell(cell)) {
           ++pixelCount;
           std::cout << pixel.x << ',' << pixel.y << ',' << pixel.z << '\n';
       }
   }

This avoids scanning the entire lattice when only the pixels belonging to a
known cell are needed.

Iterating over boundary pixels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The boundary pixel iterator provides the subset of a cell's pixels that lie
on its boundary:

.. code-block:: cpp

   for (auto cell : ctx->cells) {
       std::size_t boundaryPixelCount = 0;
       for (auto pixel : ctx->boundaryPixels.forCell(cell)) {
           ++boundaryPixelCount;
       }
   }

Boundary iteration is useful for measurements and for operations that should
affect only the cell interface.

Inspecting neighboring cells and contact area
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The neighbor iterator reports neighboring cells and their common surface:

.. code-block:: cpp

   for (auto cell : ctx->cells) {
       for (auto neighbor : ctx->neighbors.forCell(cell)) {
           if (neighbor.cell) {
               std::cout << "cell=" << cell.id
                         << " neighbor=" << neighbor.cell.id
                         << " contact=" << neighbor.commonSurfaceArea
                         << '\n';
           }
       }
   }

The medium is represented separately from a neighboring cell, so code should
handle medium contact explicitly when it matters to the calculation.

Working with fields
-------------------

Reading and writing a scalar field
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Existing scalar fields can be obtained by name. The field value wrapper can
be converted to a number for reading and assigned to for writing:

.. code-block:: cpp

   auto field = ctx->scalarFields["FGF"];
   if (field) {
       const double concentration = field(10, 20, 0);
       field(10, 20, 0) = static_cast<float>(concentration + 1.0);
   }

The field also reports its dimensions:

.. code-block:: cpp

   if (field) {
       const auto dim = field.dim();
       for (int z = 0; z < dim.z; ++z) {
           for (int y = 0; y < dim.y; ++y) {
               for (int x = 0; x < dim.x; ++x) {
                   field(x, y, z) += 0.1f;
               }
           }
       }
   }

Always check the field handle before using it. A simulation may omit a field
that a compiled module expects.

Sharing scalar and vector fields with Python
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The shared-field API allows C++ and Python steppables to use the same named
scalar or vector field. A C++ module can create fields during ``init``:

.. code-block:: cpp

   scalar_ = ctx->sharedScalarFields.createScalar(
       "cpp_scalar", CC3D_FIELD_FLOAT32, 0);

   vector_ = ctx->sharedVectorFields.createVector(
       "cpp_vector", CC3D_FIELD_FLOAT32);

The C++ side can write values as well as read them:

.. code-block:: cpp

   scalar_.set(10, 20, 0, 42.0);
   vector_.set(10, 20, 0, 1.0, 2.0, 3.0);

The Python side accesses these fields through ``self.field.cpp_scalar`` and
``self.field.cpp_vector``. This makes shared fields useful for exchanging
images, masks, concentrations, and vector-valued measurements.

Secreting chemical fields
~~~~~~~~~~~~~~~~~~~~~~~~~

A field secretor provides operations that place material inside a cell, at a
boundary, outside a boundary, or at the cell center of mass:

.. code-block:: cpp

   auto secretor = ctx->secretion.fieldSecretor("ATTR");

   for (auto cell : ctx->cells) {
       secretor.secreteInsideCell(cell, 300.0f);
       secretor.secreteInsideCellAtBoundary(cell, 300.0f);
       secretor.secreteOutsideCellAtBoundary(cell, 500.0f);
       secretor.secreteInsideCellAtCOM(cell, 300.0f);
   }

Contact-restricted variants accept a list of cell types. Total-count variants
return information about the amount actually deposited and are useful when a
steppable needs to track secretion results.

Exchanging data with Python
---------------------------

Using shared Python data
~~~~~~~~~~~~~~~~~~~~~~~~

The shared dictionary API exchanges simple values and containers with Python:

.. code-block:: cpp

   auto shared = ctx->sharedData.sharedData(&ctx->dictionaryQueries);
   shared.setDouble("cpp_to_python", static_cast<double>(ctx->mcs));

   double pythonValue = 0.0;
   if (shared.getDouble("python_to_cpp", pythonValue)) {
       shared.setDouble("cpp_seen_python_value", pythonValue);
   }

The same interface supports strings, integers, lists, tuples, booleans, and
``None``. Use explicit names for values exchanged between the two languages.

Cell-specific Python attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The cell attribute interface stores Python-compatible values on individual
cells. A steppable can set an attribute and later retrieve it after finding
the cell by ID or type:

.. code-block:: cpp

   auto attributes = ctx->cellAttributes.forCell(
       cell, &ctx->dictionaryQueries);
   attributes.setString("state", "active");
   attributes.setDouble("score", 1.5);

This is useful when a Python steppable and a C++ steppable need to associate
metadata with the same cell.

Updating XML values
~~~~~~~~~~~~~~~~~~~

XML handles are resolved by element ID. A steppable can read or update values
while the simulation is running:

.. code-block:: cpp

   auto temperature = ctx->xml.getElement("temp");
   const double value = temperature.getValueAsDouble("cdata");
   temperature.setValue("cdata", value + 1.0);

Use stable XML element IDs and keep the XML structure documented with the
simulation that uses the compiled library.

Common simulation operations
----------------------------

Reading and modifying chemotaxis data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The chemotaxis wrapper lets a steppable add and inspect chemotaxis data for a
cell:

.. code-block:: cpp

   auto chemotaxis = ctx->chemotaxis.chemotaxis();
   auto data = chemotaxis.add(cell, "ATTR");
   data.setLambda(10.0f);

The exact fields used by the wrapper depend on the configured chemotaxis
plugin and field.

Using an SBML model
~~~~~~~~~~~~~~~~~~~

SBML models attached to cells can be accessed through the SBML service:

.. code-block:: cpp

   auto model = ctx->sbml.find(cell, "OSCIL");
   if (model) {
       const double value = model.get("X");
       model.set("X", value + 0.1);
       model.timestep(0.1);
   }

The model name, variable names, and timestep must match the simulation's SBML
configuration.

Performing mitosis
~~~~~~~~~~~~~~~~~~

The mitosis service divides a cell and exposes the resulting parent and child
handles:

.. code-block:: cpp

   auto mitosis = ctx->mitosis.mitosis();
   mitosis.setParentChildPositionFlag(1);
   mitosis.divideRandom(cell);

   auto parent = mitosis.parentCell();
   auto child = mitosis.childCell();
   if (parent && child) {
       mitosis.cloneAttributes(parent, child);
   }

Mitosis changes the cell inventory, so subsequent code should use the handles
returned by the mitosis service rather than assume that the original cell is
the only valid handle.

Working with focal point plasticity links
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The focal point plasticity service can create and inspect links between cells:

.. code-block:: cpp

   auto fpp = ctx->focalPointPlasticityQueries.focalPointPlasticity(
       &ctx->focalPointPlasticity);
   fpp.create(cell, otherCell, 10.0, 2.0, 1.0);

The query interface also provides link counts and iterators for regular links,
internal links, and anchor points. This is best used when the simulation has
already configured the corresponding focal point plasticity plugin.

Advanced plugin interfaces
--------------------------

The repository includes focused examples for additional plugin services:

* cell manipulation: ``CellManipulationCompiledDemo``
* adhesion flex: ``AdhesionFlexCompiledDemo``
* cluster surface and cluster surface tracking:
  ``ClusterSurfaceCompiledDemo`` and ``ClusterSurfaceTrackerCompiledDemo``
* length constraint: ``LengthConstraintCompiledDemo``
* moment of inertia: ``MomentOfInertiaCompiledDemo``
* oriented growth: ``OrientedGrowthCompiledDemo``
* polarization vectors: ``PolarizationVectorCompiledDemo``

These examples follow the same pattern: obtain the wrapper from the kernel
context, read or update one property, and use the result in a cell operation.
They are kept as separate demos because their available parameters depend on
the plugins configured by the simulation.

Runtime discovery
~~~~~~~~~~~~~~~~~

Optional functionality can be guarded by checking whether a plugin or
steppable is registered:

.. code-block:: cpp

   if (ctx->discovery.hasPlugin("Chemotaxis")) {
       // It is safe to use the corresponding service.
   }

The discovery service can also list loaded plugins and steppables. The full
example is available in ``RuntimeDiscoveryABIDemo``.

Related examples
----------------

The source repository contains complete class-based versions of these demos.
They follow the same class-plus-C-ABI-adapter structure shown in the
quickstart. The examples in this page are intentionally shorter: each one
shows one kernel operation that can be inserted into a larger steppable.


