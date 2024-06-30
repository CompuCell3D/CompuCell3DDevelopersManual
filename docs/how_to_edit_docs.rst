Tips for Writing Documenation
========================================

How to Test Documenation
--------------------------------------

You can test your documentation in one of two ways. 
For one, you can install a VSCode Extension like "RST Preview."
Note that it can display basic things like bulleted lists and images, but it breaks for math equations. 
Alternatively, you could run the following command:

.. code-block:: console

    cd C:\...\PythonScriptingManual

    sphinx-build -a docs /tmp/mydocs

    C:\tmp\mydocs\index.html

This translates all of the RST files into HTML files and copies them to ``tmp\``. 
Open any HTML file in your browser to preview it. 
You can delete this directory when you are done. 


How to Name a Documentation File
--------------------------------------

We have a few naming conventions:

1. **section_tutorials**: The "section" prefix signals that this is a ``toctree`` file used to organize other pages.

2. **example_mitosis**: As the name implies, the file should focus on a single reproducible example rather than reference information.

3. **ref_volume**: This file would hold quick reference but nothing too detailed.

How to Write a Heading
--------------------------------------

At the top, link **Related Examples** if there are any. 
Additionally, link **Related Pages** when topics are very similar.
Finally, you can write Learning Objectives (if the doc is formatted like a lesson).


More Tips
--------------------------------------

    - Adjust the maximum column width of your IDE for ``.rst`` files to make it easier to read long lines. Long lines will word-wrap automatically.

    - Install the Grammarly extension for VSCode. 
