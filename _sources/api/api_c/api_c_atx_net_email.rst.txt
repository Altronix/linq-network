.. include:: ../../roles.rst
.. _ref_api_c_atx_net_email:

atx_net_email
===============

.. code-block:: c

   typedef struct atx_net_email_s
   {
       atx_str to0;
       atx_str to1;
       atx_str to2;
       atx_str to3;
       atx_str to4;
       atx_str from;
       atx_str subject;
       atx_str user;
       atx_str password;
       atx_str server;
       atx_str port;
       atx_str device;
       char* data;
   } atx_net_email_s;


When a device connected to the LinQ Network Library generates an alert message, the following data structure is passed to the application.

Members
-------

atx_str to[0..4]
~~~~~~~~~~~~~~~~~

Destination email address *(ie john.smith@office.com)*

atx_str from
~~~~~~~~~~~~~

Source email address

atx_str subject
~~~~~~~~~~~~~~~~

Email Subject Line

atx_str user
~~~~~~~~~~~~~

Account holder of email service provider

atx_str password
~~~~~~~~~~~~~~~~~

Password for user of email service provider

atx_str server
~~~~~~~~~~~~~~~

SMTP server IP address

atx_str port
~~~~~~~~~~~~~

SMTP Port number

atx_str device
~~~~~~~~~~~~~~~

Serial number of the device

.. rubric:: See Also

:ref:`ref_api_c_atx_net_alert`
