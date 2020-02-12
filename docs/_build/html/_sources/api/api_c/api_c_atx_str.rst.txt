.. include:: ../../roles.rst
.. _ref_api_c_atx_str:

atx_str
========

.. code-block:: c

   typedef struct atx_str {
      const char* p;
      uint32_t len;
   } atx_str;

Members
-------

const char* p
~~~~~~~~~~~~~

   Pointer to a byte array of const data

uint32_t len
~~~~~~~~~~~~

   Number of valid bytes that const char* p is pointing too.


Example
-------

.. code-block:: c

   static void
   on_alert(
       void* ctx,
       linq_network_alert_s* alert,
       linq_network_email_s* mail,
       device_s** d)
   {
      // Print out the alert message using format string
      printf("%.*s", alert->mesg.len, alert->mesg.p);
   }
   
   
.. rubric:: See Also

:ref:`ref_api_c_linq_network_alert`
