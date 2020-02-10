.. _ref_api_nodejs_atxnet_node_count:

node_count
==========

Synopsis
--------

.. code-block:: javascript

   nodeCount(): number

return how many LinQ Network Nodes the LinQ Network Library is connected to.

Parameters
----------

None.

Example
-------

.. code-block:: javascript

   let netw = require("@altronix/atx-net-js").default;

   netw.connect("ipc://local");

   console.log("Connected to %d nodes", netw.nodeCount()); // 1

.. rubric:: See Also

1. :ref:`ref_api_nodejs_atxnet_device_count`
