.. include:: ../../roles.rst
.. _ref_api_c_linq_netw_callbacks:

linq_netw_callbacks
===================

.. c:type:: linq_netw_callbacks

.. code-block:: c

   typedef struct linq_netw_callbacks
   {
       linq_netw_error_fn err;
       linq_netw_heartbeat_fn hb;
       linq_netw_alert_fn alert;
       linq_netw_ctrlc_fn ctrlc;
   } linq_netw_callbacks;

Members
-------

linq_netw_error_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_netw_error_fn)(void* context, 
                                         E_LINQ_ERROR error, 
                                         const char* description, 
                                         const char* serial);
   
   A function pointer to be called when the LinQ Network Library has detected an error.

linq_netw_heartbeat_fn
~~~~~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_netw_heartbeat_fn)(void *context,
                                             const char *serial,
                                             device_s** device_p);
   
   A function pointer to be called when the LinQ Network Libary has received a heartbeat from a LinQ enabled product.
   
linq_netw_alert_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_netw_alert_fn)(void *context,
                                         linq_netw_alert_s *alert,
                                         linq_netw_email_s *email,
                                         device_s** device_p);
   
   A function pointer to be called when the LinQ Network Libary has received an alert from a LinQ enabled product.

linq_netw_ctrlc_fn
~~~~~~~~~~~~~~~~~~

   .. code-block:: c
   
      typedef void (*linq_netw_ctrlc_fn)(void* context)
   
   A function pointer to be called when the LinQ Network Library is shutting down.
   
   
   
.. rubric:: See Also

:ref:`ref_api_c_linq_netw_create`

:ref:`ref_api_c_linq_netw_alert`

:ref:`ref_api_c_linq_netw_email`
