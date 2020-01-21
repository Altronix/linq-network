.. _ref_api_cpp_atx_net_on_alert:

on_alert
========

Synopsis
--------

Install a function pointer to be called when LinQ Network detects an alert from a device.

Parameters
----------

==================================================================== ===========
Parameter                                                            Description
==================================================================== ===========
std::function<void(atx_net_alert_s*, atx_net_email_s*, Device&)> fn) callback function
==================================================================== ===========


Example
-------

.. code-block:: cpp

   altronix::AtxNet net{};

   net.on_alert([=](atx_net_alert_s* alert, atx_net_email_s* email, altronix::Device& device) {
       std::cout << "Received alert located [" << alert.where.p << "]\n";
   });

.. rubric:: See Also

1. :ref:`ref_api_c_atx_net_alert`

2. :ref:`ref_api_c_atx_net_email`
