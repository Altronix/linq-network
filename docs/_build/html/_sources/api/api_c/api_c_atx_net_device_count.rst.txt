.. include:: ../../roles.rst
.. _ref_api_c_atx_net_device_count:

atx_net_device_count
======================

Synopsis
--------

.. c:function:: uint32_t atx_net_device_count(atx_net\* netw)

   Return the number of devices connected to your LinQ Network context.

   :param atx_net\*: Main context of LinQ Network Library required for all routines.

Examples
--------

.. rubric:: Example printing number of devices connected to the terminal.

.. code-block:: c

   atx_net* netw = atx_net_create(NULL, NULL);
   assert(netw);

   printf("There are %d devices connected to LinQ Network\n", atx_net_device_count(netw);
