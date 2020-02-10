.. _ref_api_nodejs_atxnet_run:

run
===

Synopsis
--------

.. code-block:: javascript

   run(ms: number)

The NodeJS runtime environment is a single threaded event loop. The LinQ Network Library operates in this environment with a non blocking poll routine to process network IO. To periodically allow LinQ Network Library to process network IO, call the run function with a value in milliseconds for how frequently to poll network sockets.

Parameters
----------

================ ===========
Parameter        Description
================ ===========
ms: number       Frequency in milliseconds to process network IO
================ ===========


Example
-------

.. code-block:: javascript

   let netw = require("@altronix/atx-net-js").default;

   netw.run(50);
