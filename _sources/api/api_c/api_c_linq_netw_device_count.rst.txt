.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_device_count:

linq_netw_device_count
======================

Synopsis
--------

.. c:function:: uint32_t linq_netw_device_count(linq_netw\* netw)

   Return the number of devices connected to your LinQ Network context.

   :param linq_netw\*: Main context of LinQ Network Library required for all routines.

Examples
--------

.. rubric:: Example printing number of devices connected to the terminal.

.. code-block:: c

   linq_netw* netw = linq_netw_create(NULL, NULL);
   assert(netw);

   printf("There are %d devices connected to LinQ Network\n", linq_netw_device_count(netw);
