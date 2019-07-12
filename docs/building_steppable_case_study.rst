Building Steppable
==================

It is probably best to start discussing extension of CC3D by showing a relatively sinple example of a steppable written in C++.
In typical scenario steppables are written in Python. There are three main reasons for that **1)** No compilation is required.
**2)** The code is more compact and easier to write that C++. **3)** Python has a rich set of libraries that make scientific computation
easily accessible.
However, writing a steppable in C++ is not that much more difficult, as you will see shortly, and you are almost guaranteed
that your code will run orders of magnitude faster.
Let me rephrase this last sentence - a **typical** code written in C++ is orders of magnitude faster than equivalent code written in
pure Python. Since most of the steppable code consists of iterating over all cells and adjusting their attributes, C++ will
perform this task much faster.

Getting started
---------------

Before you start developing CC3D C++ extension modules, you need to clone CC3D repository.

.. code-block:: console

    mkdir CC3D_DEVELOP
    cd CC3D_DEVELOP
    git clone https://github.com/CompuCell3D/CompuCell3D.git .

It is optional to checkout a particular branch of CC3D, but most often you will work with ``master`` branch. If ,
however, you want to checkout a branch - you woudl type something like this:

.. code-block:: console

    git checkout 4.0.0

At this point you have complete code in ``CC3D_DEVELOP`` directory. And in addition

Now we open Twedit++ - you need to have "standard" installation of CC3D on your machine available.




