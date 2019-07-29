Adding Python Bindings To Steppable in DeveloperZone
======================================================

In the previous example we controled entire simulation from C++. This is perfectly fine and will give you optimal
performance. However sometimes it may make sense to add Python bindings to your module . Especially if the
functions you wil call from python will not be called many times - functions calls in Python are much slower than in C++.

In addition to this if your entire code is in C++ every change you make to the code will require compilation and
installation. This is is not a big deal but takes time and is more error prone than using well designed scripting
interface. However, do not feel that you need to use Python bindings for your newly created C++ modules. They are
optional and it is perfectly fine to operate in C++ space.

nevertheless we would like to show you how to add python bindings if you feel it will be beneficial for your simulation