.. include:: ../../roles.rst
.. _ref_api_c_linq_network_device_count:

linq_network_device_count
======================

Synopsis
--------

.. code-block:: c

   uint32_t linq_network_device_count(const linq_network_s*);


Return the number of devices connected to your LinQ Network context.

Parameters
----------

========== ===========
Parameter  Description
========== ===========
linq_network_s* Main context of the LinQ Network Library passed to all linq_network routines.
========== ===========

Examples
--------

.. rubric:: Example printing number of devices connected to the terminal.

.. code-block:: c

   linq_network* netw = linq_network_create(NULL, NULL);
   assert(netw);

   printf("There are %d devices connected to LinQ Network\n", linq_network_device_count(netw);
