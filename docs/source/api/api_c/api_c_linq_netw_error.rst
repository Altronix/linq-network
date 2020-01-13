linq_netw_error
---------------

.. c:type:: linq_netw_error;

Many LinQ Network Library routines return a status code representing the result of the routines execution. The status code is defined by the linq_netw_error enum.

.. c:member:: linq_netw_error LINQ_ERROR_OK;

No error has occured.

.. c:member:: linq_netw_error LINQ_ERROR_OOM;

The function could not complete as intended because the system is (O)ut (O)f (M)emory.
