.. _ref_api_rust_arc_context_node_count:

arc::Context::node_count
========================

.. code-block:: rust
   
   pub fn node_count(&self) -> u32;

Return the number of nodes connected to your LinQ Network context. Any time you connect to a remote atx_net instance, the node count will increase by 1. Anytime you close a connection to a remote atx_net instance, the node count will decrease by 1.

Parameters
----------

None.

Examples
--------

.. rubric:: Example keeping track of how many nodes your application is connected to.

.. code-block:: rust

   let net = atx_net::arc::Context::new();

   assert!(net.node_count(netw) == 0);

   atx_net_socket tcp = net.connect(atx_net::Endpoint::Tcp(33455));
   
   assert!(net.node_count(netw) == 1);

   atx_net_socket ipc = net.connect(atx_net::Endpoint::Ipc("/local"));

   assert!(net.node_count(netw) == 2);

   atx_net_close(netw, ipc);

   assert!(net.node_count(netw) == 1);

   atx_net_close(netw, tcp);

   assert!(net.node_count(netw) == 0);

.. rubric:: See also

1. :ref:`ref_api_rust_arc_context_connect`
