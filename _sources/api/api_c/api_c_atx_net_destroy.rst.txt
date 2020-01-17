.. include:: ../../roles.rst
.. _ref_api_c_atx_net_destroy:

atx_net_destroy
=================

Synopsis
--------

.. c:function:: void atx_net_destroy(atx_net**)

   Free the LinQ Network communication library resource from heap memory.

   .. warning:: This will shutdown all connections to remote devices.

   :param atx_net**: Pointer to the atx_net context


Example
-------

.. code-block:: c

   atx_net *netw = atx_net_create(NULL, NULL);

   // Do some stuff
   // ...

   atx_net_destroy(&netw);

.. rubric:: See Also

:ref:`atx_net_create <ref_api_c_atx_net_destroy>`
