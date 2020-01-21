.. _ref_api_cpp_atx_net_get:

get
===

Synopsis
--------

Send a GET request to a device

Parameters
----------

================================= ============
Parameter                         Description
================================= ============
std::string serial                Serial number of the product for which to receive the request
std::string path                  URL of the request
std::function<void(Response&)> fn Calback function with response from the request
================================= ============

Example
-------

.. rubric:: Requst about data

.. code-block:: cpp

   altronix::AtxNet net{};

   net.get(
       "B3445ED2CDr2AC33298CXdd443",
       "/ATX/about",
       [=](altronix::Response& response) {
           if(response.error != LINQ_ERROR_OK) {
             std::cout << "Received error!\n" << response.error;
           }else {
             std::cout << response.response << "\n";
           }
   });

.. rubric:: See Also

1. :ref:`ref_api_cpp_atx_net_send`
