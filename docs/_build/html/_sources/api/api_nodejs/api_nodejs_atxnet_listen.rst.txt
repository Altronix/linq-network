.. _ref_api_nodejs_atxnet_listen:

listen
======

Synopsis
--------

.. code-block:: javascript

   listen(port: string) : socket

This routine will listen for incoming connections via IPC, or TCP depending on the format string passed as the first argument.  Using the listen and close API's you can dynamically change the port numbers you are listening to during runtime.

Parameters
----------

============== ===========
Parameter      Description
============== ===========
port: string   String describing the endpoint the LinQ Network Library should listen on.
============== ===========


.. rubric:: Example Endpoints

======== ============== ===========
Endpoint Example        Description
======== ============== ===========
TCP      tcp://\*:33455 Listen incoming connections on the network port 33455
HTTP     http://\*:8080 Listen for incoming HTTP connections on port 8080
IPC      ipc://local    Listen for incoming connections on IPC "local" of the current working directory of the running application.
======== ============== ===========

.. rubric:: See Also

1. :ref:`ref_api_nodejs_atxnet_connect`

2. :ref:`ref_api_nodejs_atxnet_close`
