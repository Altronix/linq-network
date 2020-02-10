.. _ref_api_nodejs_atxnet_post:

post
====

Synopsis
--------

.. code-block:: javascript

   post<T>(serial: string, path: string, data: T) : Promise<Response>

Send a POST request to a LinQ device connected to LinQ Network Library. This routine is basically a thin wrapper around :ref:`ref_api_nodejs_atxnet_send`.


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

1. :ref:`ref_api_nodejs_atxnet_get`

2. :ref:`ref_api_nodejs_atxnet_delete`

3. :ref:`ref_api_nodejs_atxnet_send`
