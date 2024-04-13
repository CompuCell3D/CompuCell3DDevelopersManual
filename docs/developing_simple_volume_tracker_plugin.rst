Simple Volume Tracker in C++
============================

The purpose of this tutorial is to show you a simplified version of the real Volume Tracker that is implemented in the CompuCell3D. We will guide you step by step how to develop the code. ANd later show you how to compile it and how to use it in a simulation.
Note that this will be rather a toy module because real VolumeTracker is loaded with every simulation we cannot have two modules incrementing and decrementing volumes . our version will only do printouts each time the cell's volume is incremented or decremented.  The reason we begin with this toy example is because it is probably the simples lattice monitor plugin one can write.

We assume that you cloned CompuCell3D repository to ``~/src-cc3d/CompuCell3D``

To begin, lets launch Twedit++. Then from CC3D C++ Menu lets select ``Generate New Module...`` and you should see the dialog:

|svp_001|

We need to fill module name, point to the directory where the code should be generated and select if the code should be
generated in Developer Zone or in the main code layout. We will pick main layout, call module ``SimpleVolumeTracker``
and pick ``/Users/m/src-cc3d/CompuCell3D/CompuCell3D/core/CompuCell3D/plugins`` as a module directory. The Module Type is ``Plugin`` and we pick ``LatticeMonitor`` as a Plugin functionality

|svp_002|

After, we click ``OK`` Twedit++ will generate code template that compiles out-of-the-box but does nothing interesting. The important thing is that Twedit++ generates a lot of boiler-plate code that we would need to write manually. This is a very tedious task and by using Twedit++ we saved ourselves a lot of work.

|svp_003|

It is our task to make this code useful. We will do it now. Here is the template of the core function (``field3DChange``) that Each Lattice Monitor needs to implement.

.. code-block:: cpp

    void SimpleVolumeTrackerPlugin::field3DChange(const Point3D &pt, CellG *newCell, CellG *oldCell)

    {
        //This function will be called after each successful pixel copy - field3DChange does usual housekeeping tasks to make sure state of cells, and state of the lattice is update

        if (newCell){
            //PUT YOUR CODE HERE
        }else{
            //PUT YOUR CODE HERE
        }

        if (oldCell){
            //PUT YOUR CODE HERE
        }else{
            //PUT YOUR CODE HERE
        }
    }


Here is how we can modify it:

.. code-block:: cpp

    void SimpleVolumeTrackerPlugin::field3DChange(const Point3D &pt, CellG *newCell, CellG *oldCell)

    {
        //This function will be called after each successful pixel copy - field3DChange does usual housekeeping tasks to make sure state of cells, and state of the lattice is update

        if (newCell){
            cerr<<"Cell id "<<newCell->id<<" increases volume by 1"<<endl;
        }else{
            cerr<<"Medium - source cell overwrites another cell's voxel"<<endl;
        }

        if (oldCell){
            cerr<<"Cell id "<<oldCell->id<<" decreases volume by 1"<<endl;
        }else{
            cerr<<"Medium - target cell's voxel gets overwritten by another cell"<<endl;
        }
    }


Here is how this code works:

1) ``field3DChange`` will be called after ``newCell`` overwrites pixel occupied by the ``oldCell``. This call is triggered automatically from the ``metropolisFast`` function. How does metropolisFast know which lattice monitory to call - we will cover it soon.

2) It may happen that pointer to ``newCell`` or ``oldCell`` may be NULL which corresponds to situation where either of these cells is Medium. We need to handle this case so that we do not attempt to access ``id`` or ``volume`` attribute of the NULL pointer.

3) Since ``newCell`` is the cell that overwrites another cell - it is the cell whose volume increases. By analogy, oldCell is the one that is overwritten, hence loses one pixel.

4) the else portion of the code handles situation where we are dealing with Medium. Medium is represented in CC3D as a ``null`` pointer and hence we need to handle it as such

Let us compile the project. Since we have already did our initial setup involving cmake all we need to do is to run ``make`` and  ``make install``

.. code-block:: console

    cd /Users/m/src-cc3d/CompuCell3D_build
    make -j 8
    make install

This next step is only required if we modified core CC3D files like Potts.cpp or Simulator.cpp If we worked on plugin code only we can skip it

.. code-block:: console

    cp /Users/m/src-cc3d/CompuCell3D_install/lib/*.dylib /Users/m/miniconda3_arm64/envs/cc3d_compile/lib



.. note::

    Notice that if we did any modification to ``CMakeLists.txt`` files (and Twedit++ did ir for us during plugin code generation) the first thing that happens when we run ``make`` command is reconfiguration of the entire cmake Build system but fortunately thi sis done automatically. We only need to call cmake command once , when we first det up the compilation of CompuCell3D. The only downside on Mac is that this reconfiguration of the cmake build system for our compilation directory of CC3D will cause recompilation of the entire CompuCell3D. However, this will only happen if add new module or modify CmakeLists.txt files. If we change the C++ code for the plugin only the plugin should get compiled

After we compile this plugin (see materials on how to compile CC3D on your platform) we can use it in our simulation. Insert the following

.. code-block:: xml

    <Plugin Name="SimpleVolumeTracker"/>

inside XML  - in my case  ``/Users/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/Simulation/cellsort_2D.xml``:

.. code-block:: xml

    <CompuCell3D>
        <Potts>
            <Dimensions x="100" y="100" z="1"/>
            <Anneal>10</Anneal>
            <Steps>10000</Steps>
            <Temperature>10</Temperature>
            <Flip2DimRatio>1</Flip2DimRatio>
            <NeighborOrder>2</NeighborOrder>
        </Potts>


        <Plugin Name="Volume">
            <TargetVolume>25</TargetVolume>
            <LambdaVolume>2.0</LambdaVolume>
        </Plugin>

        <Plugin Name="CellType">
            <CellType TypeName="Medium" TypeId="0"/>
            <CellType TypeName="Condensing" TypeId="1"/>
            <CellType TypeName="NonCondensing" TypeId="2"/>
        </Plugin>

        <Plugin Name="Contact">
            <Energy Type1="Medium" Type2="Medium">0</Energy>
            <Energy Type1="NonCondensing" Type2="NonCondensing">16</Energy>
            <Energy Type1="Condensing" Type2="Condensing">2</Energy>
            <Energy Type1="NonCondensing" Type2="Condensing">11</Energy>
            <Energy Type1="NonCondensing" Type2="Medium">16</Energy>
            <Energy Type1="Condensing" Type2="Medium">16</Energy>
            <NeighborOrder>2</NeighborOrder>
        </Plugin>

        <Plugin Name="SimpleVolumeTracker"/>

        <Steppable Type="BlobInitializer">

            <Region>
                <Center x="50" y="50" z="0"/>
                <Radius>40</Radius>
                <Gap>0</Gap>
                <Width>5</Width>
                <Types>Condensing,NonCondensing</Types>
            </Region>
        </Steppable>


    </CompuCell3D>


then run it using

.. code-block::

    python -m cc3d.run_script -i /Users/m/src-cc3d/CompuCell3D/CompuCell3D/core/Demos/Models/cellsort/cellsort_2D/cellsort_2D.cc3d


and we should get printouts printouts that look as follows:

.. code-block:: console

    Cell id 148 decreases volume by 1
    Cell id 149 increases volume by 1
    Cell id 162 decreases volume by 1
    Cell id 83 increases volume by 1
    Cell id 82 decreases volume by 1
    Cell id 189 increases volume by 1
    Cell id 188 decreases volume by 1

Congratulations. You have developed your first CompuCell3D plugins. Even though the SimpleVolumeTracker in its current form is not terribly useful it taught us the mechanics of adding new plugin, compiling cc3d and using new plugin with freshly compiled CC3D. In the next chapters we will develop more pragmatic examples


.. |svp_001| image:: images/simple_volume_tracker_001.png

.. |svp_002| image:: images/simple_volume_tracker_002.png

.. |svp_003| image:: images/simple_volume_tracker_003.png