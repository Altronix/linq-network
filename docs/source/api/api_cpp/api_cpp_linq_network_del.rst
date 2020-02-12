.. _ref_api_cpp_linq_network_del:

del
===

Synopsis
--------

Send a DELETE request to a device

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

.. rubric:: Delete a user from the device

.. code-block:: cpp

   altronix::AtxNet net{};

   net.del(
       "B3445ED2CDr2AC33298CXdd443",
       "/ATX/userManagement/users/john",
       [=](altronix::Response& response) {
           if(response.error != LINQ_ERROR_OK) {
             std::cout << "Received error!\n" << response.error;
           }else {
             std::cout << response.response << "\n";
           }
   });

.. rubric:: See Also

1. :ref:`ref_api_cpp_linq_network_send`
