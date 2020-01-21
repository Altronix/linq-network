.. _ref_api_cpp_atx_net_on_ctrlc:

on_ctrlc
========

Synopsis
--------

Install a function pointer to be called when LinQ Network is shutting down.

Parameters
----------

========================== ===========
Parameter                  Description
========================== ===========
std::function<void()> fn)  callback function
========================== ===========


Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   net.on_ctrlc([=]() {
       std::cout << "LinQ Network Shutting Down...\n";
   });
