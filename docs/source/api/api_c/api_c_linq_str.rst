.. include:: ../../roles.rst
.. _ref_api_c_linq_str:

linq_str
========

.. code-block:: c

   typedef struct linq_str {
      const char* p;
      uint32_t len;
   } linq_str;

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
       atx_net_alert_s* alert,
       atx_net_email_s* mail,
       device_s** d)
   {
      // Print out the alert message using format string
      printf("%.*s", alert->mesg.len, alert->mesg.p);
   }
   
   
.. rubric:: See Also

:ref:`ref_api_c_atx_net_alert`
