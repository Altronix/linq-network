.. include:: ../../roles.rst
.. _ref_api_c_atx_net_node_count:

atx_net_node_count
======================

Synopsis
--------

.. c:function:: uint32_t atx_net_node_count(atx_net\* netw)

   Return the number of nodes connected to your LinQ Network context.

   :param atx_net\*: Main context of LinQ Network Library required for all routines.

Examples
--------

.. rubric:: Example keeping track of how many nodes your application is connected to.

.. code-block:: c

   atx_net* netw = atx_net_create(NULL, NULL);
   assert(netw);

   assert(atx_net_node_count(netw) == 0);

   atx_net_socket tcp = atx_net_connect(netw, "tcp://127.0.0.1:33455");
   
   assert(atx_net_node_count(netw) == 1);

   atx_net_socket ipc = atx_net_connect(netw, "ipc://channel");

   assert(atx_net_node_count(netw) == 2);

   atx_net_close(netw, ipc);

   assert(atx_net_node_count(netw) == 1);

   atx_net_close(netw, tcp);

   assert(atx_net_node_count(netw) == 0);
