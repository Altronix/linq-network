.. include:: ../../roles.rst
.. _ref_api_c_atx_net_callbacks:

atx_net_callbacks
===================

.. c:type:: atx_net_callbacks

.. code-block:: c

   typedef struct atx_net_callbacks
   {
       atx_net_error_fn err;
       atx_net_heartbeat_fn hb;
       atx_net_alert_fn alert;
       atx_net_ctrlc_fn ctrlc;
   } atx_net_callbacks;

Members
-------

atx_net_error_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*atx_net_error_fn)(void* context, 
                                         E_LINQ_ERROR error, 
                                         const char* description, 
                                         const char* serial);
   
   A function pointer to be called when the LinQ Network Library has detected an error.

atx_net_heartbeat_fn
~~~~~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*atx_net_heartbeat_fn)(void *context,
                                             const char *serial,
                                             device_s** device_p);
   
   A function pointer to be called when the LinQ Network Libary has received a heartbeat from a LinQ enabled product.
   
atx_net_alert_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*atx_net_alert_fn)(void *context,
                                         atx_net_alert_s *alert,
                                         atx_net_email_s *email,
                                         device_s** device_p);
   
   A function pointer to be called when the LinQ Network Libary has received an alert from a LinQ enabled product.

atx_net_ctrlc_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*atx_net_ctrlc_fn)(void* context)
   
   A function pointer to be called when the LinQ Network Library is shutting down.
   
   
   
.. rubric:: See Also

:ref:`ref_api_c_atx_net_create`

:ref:`ref_api_c_atx_net_alert`

:ref:`ref_api_c_atx_net_email`
