.. include:: ../../roles.rst
.. _ref_api_c_linq_network_node_count:

linq_network_node_count
======================

Synopsis
--------

.. code-block:: c

   uint32_t linq_network_node_count(linq_network_s* netw);

Return the number of nodes connected to your LinQ Network context. Any time you connect to a remote linq_network instance, the node count will increase by 1. Anytime you close a connection to a remote linq_network instance, the node count will decrease by 1.

Parameters
----------

========= ===========
Parameter Description
========= ===========
linq_network   Main context of the LinQ Network Library passed to all linq_network routines.
========= ===========

Examples
--------

.. rubric:: Example keeping track of how many nodes your application is connected to.

.. code-block:: c

   linq_network* netw = linq_network_create(NULL, NULL);
   assert(netw);

   assert(linq_network_node_count(netw) == 0);

   linq_network_socket tcp = linq_network_connect(netw, "tcp://127.0.0.1:33455");
   
   assert(linq_network_node_count(netw) == 1);

   linq_network_socket ipc = linq_network_connect(netw, "ipc://channel");

   assert(linq_network_node_count(netw) == 2);

   linq_network_close(netw, ipc);

   assert(linq_network_node_count(netw) == 1);

   linq_network_close(netw, tcp);

   assert(linq_network_node_count(netw) == 0);
