.. _ref_api_cpp_atx_net_post:

post
====

Synopsis
--------

Send a POST request to a device

Parameters
----------

================================= ============
Parameter                         Description
================================= ============
std::string serial                Serial number of the product for which to receive the request
std::string path                  URL of the request
std::string data                  Body of the request
std::function<void(Response&)> fn Calback function with response from the request
================================= ============

Example
-------

.. rubric:: Requst about data

.. code-block:: cpp

   altronix::AtxNet net{};

   net.post(
       "B3445ED2CDr2AC33298CXdd443",
       "/ATX/about",
       "{\"siteId\":\"New Site Id\"}",
       [=](altronix::Response& response) {
           if(response.error != LINQ_ERROR_OK) {
             std::cout << "Received error!\n" << response.error;
           }else {
             std::cout << response.response << "\n";
           }
   });

.. rubric:: See Also

1. :ref:`ref_api_cpp_atx_net_send`
