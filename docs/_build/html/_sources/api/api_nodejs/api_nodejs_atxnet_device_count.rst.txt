.. _ref_api_nodejs_atxnet_device_count:

device_count
============

Synopsis
--------

.. code-block:: javascript

   deviceCount(): number

return how many LinQ Network Nodes the LinQ Network Library is connected to.

Parameters
----------

None.

Example
-------

.. code-block:: javascript

   let netw = require("@altronix/atx-net-js").default;

   netw.listen("ipc://local");

   console.log("Connected to %d devices", netw.deviceCount());

.. rubric:: See Also

1. :ref:`ref_api_nodejs_atxnet_node_count`
