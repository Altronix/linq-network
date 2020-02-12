.. include:: ../../roles.rst
.. _ref-api-nodejs:

NodeJS API Reference
====================

The LinQ Network Library provides a NPM package.

.. code-block:: javascript

   let linq = require("@altronix/linq-network-js").default;
   linq.listen("tcp://*:33455");

.. toctree::
   :maxdepth: 1

   api_nodejs_atxnet.rst
   api_nodejs_atxnet_listen.rst
   api_nodejs_atxnet_connect.rst
   api_nodejs_atxnet_close.rst
   api_nodejs_atxnet_get.rst
   api_nodejs_atxnet_post.rst
   api_nodejs_atxnet_delete.rst
   api_nodejs_atxnet_send.rst
   api_nodejs_atxnet_node_count.rst
   api_nodejs_atxnet_device_count.rst
   api_nodejs_atxnet_devices.rst
   api_nodejs_atxnet_on.rst
   api_nodejs_atxnet_run.rst
