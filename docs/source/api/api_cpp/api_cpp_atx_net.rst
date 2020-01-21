.. _ref_api_cpp_atx_net:

AtxNet
======

Class
-----

.. code-block:: c

   class AtxNet;

Synopsis
--------

The Altronix AtxNet C++ binding is a header only wrapper around the atx_net API that provides a simple RAII interface to the C library.

Members
-------

.. rubric:: listen

Listen for incoming connections

.. code-block:: c

   atx_net_socket listen(const char* str);

.. rubric:: connect

Connect to remote AtxNet node

.. code-block:: c

   atx_net_socket connect(const char* str);

.. rubric:: close

Close a AtxNet connection

.. code-block:: c

   void close(atx_net_socket s);

.. rubric:: poll

Process Network IO

.. code-block:: c

   E_LINQ_ERROR poll(uint32_t ms);

.. rubric:: send

Send a request to a device connected to AtxNet

.. code-block:: c

   void send(
      std::string serial
      std::string method,
      std::string path,
      std::string data,
      std::function<void(Response&)> fn);

   void send(
      std::string serial,
      std::string method,
      std::string path,
      std::function<void(Response&)> fn);

.. rubric:: get

Send a get request to a device connected to AtxNet

.. code-block:: c

   void get(
      std::string serial,
      std::string path,
      std::function<void(Response&)> fn);

.. rubric:: post

Send a post request to a device connected to AtxNet

.. code-block:: c

   void post(
      std::string serial,
      std::string path,
      std::string data,
      std::function<void(Response&)> fn);

.. rubric:: del

Send a delete request to a device connected to AtxNet

.. code-block:: c

   void del(
      std::string serial,
      std::string path,
      std::function<void(Response&)> fn);

.. rubric:: device_count

Number of devices connected to AtxNet node

.. code-block:: c

   uint32_t device_count();

.. rubric:: node_count

Number of nodes connected to AtxNet node

.. code-block:: c

   uint32_t node_count();

.. rubric:: on_heartbeat

Install function to be called every heartbeat event on the network

.. code-block:: c

   AtxNet& on_heartbeat(std::function<void(const char*, Device&)> fn);

.. rubric:: on_alert

Install function to be called every alert event on the network

.. code-block:: c

   AtxNet& on_alert(std::function<void(atx_net_alert_s*, atx_net_email_s*, Device&)> fn);

.. rubric:: on_error

Install function to be called every error event on the network

.. code-block:: c

   AtxNet& on_error(std::function<void(E_LINQ_ERROR, const char*, const char*)> fn);

.. rubric:: on_ctrlc

Install function to be called when AtxNet is shutting down

.. code-block:: c

   AtxNet& on_alert(std::function<void()> fn);
