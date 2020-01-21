.. _ref_api_cpp_atx_net_on_heartbeat:

on_heartbeat
============

Synopsis
--------

Install a function pointer to be called when LinQ Network detects a heartbeat from a device.

Parameters
----------

============================================= ===========
Parameter                                     Description
============================================= ===========
std::function<void(const char*, Device&)> fn) callback function
============================================= ===========


Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   net.on_heartbeat([=](const char* serial, altronix::Device& device) {
       std::cout << "Received heartbeat from [" << serial << "]\n";
   });
