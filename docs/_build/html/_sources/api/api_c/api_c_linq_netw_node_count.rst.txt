.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_node_count:

linq_netw_node_count
======================

Synopsis
--------

.. c:function:: uint32_t linq_netw_node_count(linq_netw\* netw)

   Return the number of nodes connected to your LinQ Network context.

   :param linq_netw\*: Main context of LinQ Network Library required for all routines.

Examples
--------

.. rubric:: Example keeping track of how many nodes your application is connected to.

.. code-block:: c

   linq_netw* netw = linq_netw_create(NULL, NULL);
   assert(netw);

   assert(linq_netw_node_count(netw) == 0);

   linq_netw_socket tcp = linq_netw_connect(netw, "tcp://127.0.0.1:33455");
   
   assert(linq_netw_node_count(netw) == 1);

   linq_netw_socket ipc = linq_netw_connect(netw, "ipc://channel");

   assert(linq_netw_node_count(netw) == 2);

   linq_netw_close_dealer(netw, ipc);

   assert(linq_netw_node_count(netw) == 1);

   linq_netw_close_dealer(netw, tcp);

   assert(linq_netw_node_count(netw) == 0);
