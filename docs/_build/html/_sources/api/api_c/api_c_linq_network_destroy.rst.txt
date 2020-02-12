.. include:: ../../roles.rst
.. _ref_api_c_linq_network_destroy:

linq_network_destroy
====================

Synopsis
--------

.. code-block:: c

   void linq_network_destroy(linq_network_s**);

Free the LinQ Network communication library resource from heap memory.

.. warning:: This will shutdown all connections to remote devices.

Parameters
----------

============== ===========
Parameter      Description
============== ===========
linq_network** Pointer to the linq_network_s context (returned from linq_network_create)
============== ===========

Example
-------

.. code-block:: c

   linq_network *netw = linq_network_create(NULL, NULL);

   // Do some stuff
   // ...

   linq_network_destroy(&netw);

.. rubric:: See Also

:ref:`linq_network_create <ref_api_c_linq_network_destroy>`
