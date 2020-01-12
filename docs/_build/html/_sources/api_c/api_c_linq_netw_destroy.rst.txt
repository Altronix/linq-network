linq_netw_destroy()
===================

.. c:function:: linq_netw_destroy(linq_netw**)

   Free the LinQ Network communication library resource from heap memory.

   .. warning:: This will shutdown all connections to remote devices.

   :param linq_netw**: Pointer to the linq_netw context

Example
-------

.. code-block:: c

   linq_netw *netw = linq_netw_create(NULL, NULL);

   // ...

   linq_netw_destroy(&netw);
