.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_destroy:

linq_netw_destroy
=================

Synopsis
--------

.. c:function:: void linq_netw_destroy(linq_netw**)

   Free the LinQ Network communication library resource from heap memory.

   .. warning:: This will shutdown all connections to remote devices.

   :param linq_netw**: Pointer to the linq_netw context


Example
-------

.. code-block:: c

   linq_netw *netw = linq_netw_create(NULL, NULL);

   // Do some stuff
   // ...

   linq_netw_destroy(&netw);

.. rubric:: See Also

:ref:`linq_netw_create <ref_api_c_linq_netw_destroy>`
