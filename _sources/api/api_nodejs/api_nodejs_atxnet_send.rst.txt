.. _ref_api_nodejs_atxnet_send:

send
====

Synopsis
--------

.. code-block:: javascript

   send<T>(serial: string, method: string, path: string, data: T): Promise<Response>

Parameters
----------

================ ===========
Parameter        Description
================ ===========
serial: string   Serial number of the device to receive request
method: string   The HTTP Verb of the Request (IE: "GET")
path: string     The URL of the Request (IE: "/ATX/about")
data: T          The body of the request
================ ===========

Example
-------

.. rubric:: Request general data from every device that sends a heartbeat and print the response on the console.

.. code-block:: javascript

   let netw = require("@altronix/atx-net-js").default;

   netw.listen("tcp://*:33455")
       .on("heartbeat", sid =>
         netw
            .send(sid, "GET", "/ATX/about")
            .then(response => console.log(response))
            .catch(e => console.log(e)));
            

.. rubric:: See Also

1. :ref:`ref_api_nodejs_atxnet_get`

2. :ref:`ref_api_nodejs_atxnet_post`

3. :ref:`ref_api_nodejs_atxnet_delete`
