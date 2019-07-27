Building Growth Steppable In the Developer Zone folder
======================================================

Quite often you will want to build a steppable in a "non-intrusive" way *i.e.* without adding it to the main
CC3D code-base. The way to do it is to utilize functionality of ``DeveloperZone``.

This time we will use Windows system and our  CC3D git repository is cloned to ``D:\CC3D_PY3_GIT\CompuCell3D``

To add a steppable or plugin in the DeveloperZone you open up Twedit and from ``CC3D C++`` menu select
``Generate New Module...``.

|dev_zone_1|

Notice that in the ``Module Directory`` in the dialog box we put ``D:\CC3D_PY3_GIT\CompuCell3D\DeveloperZone``.
Previously we put there a path to the Steppable folder in the main CC3D Code base (give where our repository is cloned
this path would be ``D:\CC3D_PY3_GIT\CompuCell3D\core\CompuCell3D\steppables\``)

Notice that we also checked ``Python Wrap`` option to generate Python bindings. We will show you how you ca
be creative here and leverage both XML and Python as a way to pass parameters to the Steppable. As you
remember you do not have to generate Python bindings and it is perfectly OK to to stick with C++ and XML.

After we press ``OK`` button Twedit++ will generate , a template Steppable code:

|dev_zone_2|

Now we copy code from our earlier example into appropriate files - we are only showing files that we modified


.. |dev_zone_1| image:: images/dev_zone_1.png
   :width: 2.4in
   :height: 1.9in

.. |dev_zone_2| image:: images/dev_zone_2.png
   :width: 6.0in
   :height: 2.5in