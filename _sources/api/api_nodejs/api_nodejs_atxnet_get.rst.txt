.. _ref_api_nodejs_atxnet_get:

get
===

Synopsis
--------

.. code-block:: javascript

   get(serial: string, path: string) : Promise<Response>

Send a GET request to a LinQ device connected to LinQ Network Library. This routine is basically a thin wrapper around :ref:`ref_api_nodejs_atxnet_send`.

Parameters
----------

================ ===========
Parameter        Description
================ ===========
serial: string   Serial number of the device to receive request
path: string     The URL of the request (IE: "/ATX/about")
================ ===========

Example
-------

See :ref:`ref_api_nodejs_atxnet_send`

.. rubric:: See Also

1. :ref:`ref_api_nodejs_atxnet_post`

2. :ref:`ref_api_nodejs_atxnet_delete`

3. :ref:`ref_api_nodejs_atxnet_send`
