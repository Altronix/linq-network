.. _ref_api_cpp_linq_network_on_alert:

on_alert
========

Synopsis
--------

Install a function pointer to be called when LinQ Network detects an alert from a device.

Parameters
----------

============================================================================== ===========
Parameter                                                                      Description
============================================================================== ===========
std::function<void(linq_network_alert_s*, linq_network_email_s*, Device&)> fn) callback function
============================================================================== ===========


Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   net.on_alert([=](linq_network_alert_s* alert, linq_network_email_s* email, altronix::Device& device) {
       std::cout << "Received alert located [" << alert.where.p << "]\n";
   });

.. rubric:: See Also

1. :ref:`ref_api_c_linq_network_alert`

2. :ref:`ref_api_c_linq_network_email`
