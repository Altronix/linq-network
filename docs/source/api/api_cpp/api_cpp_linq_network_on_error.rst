.. _ref_api_cpp_linq_network_on_error:

on_error
========

Synopsis
--------

Install a function pointer to be called when LinQ Network detects an error.

Parameters
----------

=============================================================== ===========
Parameter                                                       Description
=============================================================== ===========
std::function<void(E_LINQ_ERROR, const char*, const char*)> fn) callback function
=============================================================== ===========


Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   net.on_error([=](E_LINQ_ERROR error, const char *what, const char *serial) {
       std::cout << "Received error: " << error
                 << "from :" << serial << "\n";
   });
