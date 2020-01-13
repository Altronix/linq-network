.. include:: ../../roles.rst

:api-enum:`Enum`:api-header:`linq_netw_error`
---------------------------------------------

.. c:type:: linq_netw_error;

Many LinQ Network Library routines return a status code representing the result of the routines execution. The status code is defined by the linq_netw_error enum.

.. table::
   :align: left

   ============================= ===========
   Name                          Description
   ============================= ===========
   LINQ_ERROR_OK                 No error has occured.
   LINQ_ERROR_OOM                The function could not complete as intended because the system is (O)ut (O)f (M)emory.
   LINQ_ERROR_BAD_ARGS           Caller has called the routine with invalid parameters.
   LINQ_ERROR_IO                 The system returned an IO error (Either from file read or network IO).
   LINQ_ERROR_DEVICE_NOT_FOUND   The routine could not complete as intended because the device is not connected.
   LINQ_ERROR_TIMEOUT            The routine is taking too long to complete and has decided to return an error.
   LINQ_ERROR_SHUTTING_DOWN      The LinQ Network Library is in the process of shutting down and therefore cannot perform this routine.
   LINQ_ERROR_400                A response from the device has returned a 400 error code (Client API error)
   LINQ_ERROR_403                A response from the device has returned a 403 error code (Unauthorized)
   LINQ_ERROR_404                A response from the device has returned a 404 error code (Not Found)
   LINQ_ERROR_500                A response from the device has returned a 500 error code (Internal Server Error)
   LINQ_ERROR_504                A response from the device has returned a 504 error code (Try again later)
   ============================= ===========
