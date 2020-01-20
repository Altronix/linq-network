.. include:: ../../roles.rst
.. _ref_api_c_atx_net_device_count:

atx_net_device_count
======================

Synopsis
--------

.. code-block:: c

   uint32_t atx_net_device_count(const atx_net_s*);


Return the number of devices connected to your LinQ Network context.

Parameters
----------

========== ===========
Parameter  Description
========== ===========
atx_net_s* Main context of the LinQ Network Library passed to all atx_net routines.
========== ===========

Examples
--------

.. rubric:: Example printing number of devices connected to the terminal.

.. code-block:: c

   atx_net* netw = atx_net_create(NULL, NULL);
   assert(netw);

   printf("There are %d devices connected to LinQ Network\n", atx_net_device_count(netw);
