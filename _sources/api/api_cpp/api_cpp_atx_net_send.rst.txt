.. _ref_api_cpp_atx_net_send:

send
====

Synopsis
--------

Send a request to a device connected to the LinQ Network. Pass in a lamda argument to receive a callback when the response is ready.

Parameters
----------

================================= ============
Parameter                         Description
================================= ============
std::string serial                Serial number of the product for which to receive the request
std::string meth                  Type of request (GET, POST, DELETE)
std::string path                  URL of the request
std::string data (optional)       Body of the request
std::function<void(Response&)> fn Calback function with response from the request
================================= ============

Example
-------

.. rubric:: Requst about data

.. code-block:: cpp

   altronix::AtxNet net{};

   net.send(
       "B3445ED2CDr2AC33298CXdd443",
       "GET",
       "/ATX/about",
       [=](altronix::Response& response) {
           if(response.error != LINQ_ERROR_OK) {
             std::cout << "Received error!\n" << response.error;
           }else {
             std::cout << response.response << "\n";
           }
   });

.. rubric:: Update Site ID

.. code-block:: cpp

   altronix::AtxNet net{};

   net.send(
       net, 
       "B3445ED2CDr2AC33298CXdd443", 
       "/ATX/about", 
       "{\"siteId\": \"New Site ID\"}",
       [=](altronix::Response& response) {
           if(response.error != LINQ_ERROR_OK) {
             std::cout << "Received error!\n" << response.error;
           }else {
             std::cout << response.response << "\n";
           }
       })
   );
