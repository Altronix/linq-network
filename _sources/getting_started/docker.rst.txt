.. _ref_getting_started_docker:

Docker Image
============

The LinQ Network Library has a demonstration application that conforms to the linqd API specification :ref:`(See here) <ref_api_http_linqd>`. The LinQ Network demo application docker container can serve as an API endpoint during front end development. The following methods below can get you started with running the docker image.

.. note:: The ``altronix/linqd:demo`` container is listing to HTTP connections on port 8000 and listening for device connections on port 33455. You must use the -p flags to expose the container ports to your host network.

Running docker image from docker hub
------------------------------------

Use this method to download the docker image from docker hub and run the service on your local machine.

.. code-block:: bash

   docker run --rm -d -p 8001:8000 -p 33456:33455 altronix/linqd:demo

The following commands are described:

============== ===========
Flags          Description
============== ===========
--rm           Remove this container when the container exits
-d             Run this image in "background" (IE: Daemon mode)
-p 8001:8000   Forward incoming connections to my host machine on port 8001 to port 8000 on the ``altronix/linqd:demo`` container
-p 33456:33455 Forward incoming connections to my host machine on port 33456 to port 33455 on the ``altronix/linqd:demo`` container
============== ===========

Download the image from docker hub
----------------------------------

You can alternatively download the ``altronix/linqd:demo`` container to run at a later time

.. code-block:: bash

   docker pull altronix/linqd:demo

Then run the container

.. code-block:: bash

   docker run --rm -d -p 8000:8000 -p 33455:33455 altronix/linqd:demo

Build the docker container from source
--------------------------------------

For developers working on the LinQ Network Library or wishing to modify the docker image. You can build the docker image from source.

.. code-block:: bash

   git clone https://github.com/altronix/linq-network
   cd linq-network
   git submodule update --init
   docker build -t linqd -f docker/ubuntu-aarch64/Dockerfile .

Then run the container

.. code-block:: bash

   docker run --rm -d -p8000:8000 -p 33455:33455 linqd

When the LinQ Service Demo container is running and you have successfully exposed the container ZMTP and HTTP ports, you can connect devices to the LinQ Network Service and access device data via the :ref:`ref_api_http_linqd`
