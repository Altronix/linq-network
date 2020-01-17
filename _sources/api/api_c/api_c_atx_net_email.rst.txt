.. include:: ../../roles.rst
.. _ref_api_c_atx_net_email:

atx_net_email
===============

.. c::type atx_net_email

.. code-block:: c

   typedef struct atx_net_email_s
   {
       linq_str to0;
       linq_str to1;
       linq_str to2;
       linq_str to3;
       linq_str to4;
       linq_str from;
       linq_str subject;
       linq_str user;
       linq_str password;
       linq_str server;
       linq_str port;
       linq_str device;
       char* data;
   } atx_net_email_s;


When a device connected to the LinQ Network Library generates an alert message, the following data structure is passed to the application.

Members
-------

linq_str to[0..4]
~~~~~~~~~~~~~~~~~

Destination email address *(ie john.smith@office.com)*

linq_str from
~~~~~~~~~~~~~

Source email address

linq_str subject
~~~~~~~~~~~~~~~~

Email Subject Line

linq_str user
~~~~~~~~~~~~~

Account holder of email service provider

linq_str password
~~~~~~~~~~~~~~~~~

Password for user of email service provider

linq_str server
~~~~~~~~~~~~~~~

SMTP server IP address

linq_str port
~~~~~~~~~~~~~

SMTP Port number

linq_str device
~~~~~~~~~~~~~~~

Serial number of the device

.. rubric:: See Also

:ref:`ref_api_c_atx_net_alert`
