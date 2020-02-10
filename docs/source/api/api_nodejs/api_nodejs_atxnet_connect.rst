.. _ref_api_nodejs_atxnet_connect:

connect
=======

Synopsis
--------

.. code-block:: javascript

   connect(port: string) : socket

This routine will connect to remote node via IPC, or TCP depending on the format string passed as the first argument.  Using the connect and close API's you can dynamically change the port numbers you are connect to during runtime.

Parameters
----------

================ ===========
Parameter        Description
================ ===========
endpoint: string String describing the endpoint the LinQ Network Library should connect to.
================ ===========

.. rubric:: Example Endpoints

======== ============== ===========
Endpoint Example        Description
======== ============== ===========
TCP      tcp://\*:33455 Listen incoming connections on the network port 33455
HTTP     http://\*:8080 Listen for incoming HTTP connections on port 8080
IPC      ipc://local    Listen for incoming connections on IPC "local" of the current working directory of the running application.
======== ============== ===========

Example
-------

See :ref:`ref_api_nodejs_atxnet_listen`

.. rubric:: See Also

1. :ref:`ref_api_nodejs_atxnet_listen`

2. :ref:`ref_api_nodejs_atxnet_close`
