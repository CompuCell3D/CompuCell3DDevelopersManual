Simple Volume Tracker in C++ Part 2 - Changing Cell Type
=========================================================

Our previous example showed us how to do printouts from our newly written plugin. Now lets write code that alters cell state. We will implemement functionality that if any cell in simulation reaches volume ``27`` its type id (``cell->type`` in C++) will be changed to `1` . Because target volume of our simulation is ``25`` and we know that this volume subject to random fluctuations we should be able to turn quite a few of cells in our simulation into cells of type id `1`

.. note::

    During the course of the single MonteCarlo step a given cell may change its volume many times

We will only trigger cell type change when volume 27 or higher is reached when the cell increases its volume and therefore our code looks as follows:

.. code-block:: cpp

    void SimpleVolumeTrackerPlugin::field3DChange(const Point3D &pt, CellG *newCell, CellG *oldCell)

    {

        // note: we only care about cells that reach volume 27 when they grow
        if (newCell) {
            if (newCell->volume >= 27) {
                newCell->type = 1;
            }
        }

    }


After we compile it and run it we will see that relatively quickly most of the cells in the simulation will turn to cell type `1` (blue color)

|svp_004a|

This tiny code change concludes our next example.

.. |svp_004a| image:: images/simple_volume_tracker_004a.png

