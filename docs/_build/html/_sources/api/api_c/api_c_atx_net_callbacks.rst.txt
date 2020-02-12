.. include:: ../../roles.rst
.. _ref_api_c_linq_network_callbacks:

linq_network_callbacks
===================

.. code-block:: c

   typedef struct linq_network_callbacks
   {
       linq_network_error_fn err;
       linq_network_heartbeat_fn hb;
       linq_network_alert_fn alert;
       linq_network_ctrlc_fn ctrlc;
   } linq_network_callbacks;

Members
-------

linq_network_error_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_network_error_fn)(void* context, 
                                         E_LINQ_ERROR error, 
                                         const char* description, 
                                         const char* serial);
   
   A function pointer to be called when the LinQ Network Library has detected an error.

linq_network_heartbeat_fn
~~~~~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_network_heartbeat_fn)(void *context,
                                             const char *serial,
                                             device_s** device_p);
   
   A function pointer to be called when the LinQ Network Libary has received a heartbeat from a LinQ enabled product.
   
linq_network_alert_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_network_alert_fn)(void *context,
                                         linq_network_alert_s *alert,
                                         linq_network_email_s *email,
                                         device_s** device_p);
   
   A function pointer to be called when the LinQ Network Libary has received an alert from a LinQ enabled product.

linq_network_ctrlc_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_network_ctrlc_fn)(void* context)
   
   A function pointer to be called when the LinQ Network Library is shutting down.
   
   
   
.. rubric:: See Also

:ref:`ref_api_c_linq_network_create`

:ref:`ref_api_c_linq_network_alert`

:ref:`ref_api_c_linq_network_email`
