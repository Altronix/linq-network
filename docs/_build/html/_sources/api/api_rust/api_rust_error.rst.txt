.. _ref_api_rust_error:

NetworkErrorKind
================

Synopsis
--------

.. code-block:: rust

   #[derive(Debug, Copy, Clone, PartialEq)]
   pub enum NetworkErrorKind;

Variants
--------

================= ===========
Name              Description
================= ===========
OK                No error has occured.
OutOfMemory       The function could not complete as intended because the system is (O)ut (O)f (M)emory.
BadArgs           Caller has called the routine with invalid parameters.
Protocol          The system returned an IO error (Either from file read or network IO).
DeviceNotFound    The routine could not complete as intended because the device is not connected.
Timeout           The routine is taking too long to complete and has decided to return an error.
ShuttingDown      The LinQ Network Library is in the process of shutting down and therefore cannot perform this routine.
HttpBadRequest    A response from the device has returned a 400 error code (Client API error)
HttpUnauthorized  A response from the device has returned a 403 error code (Unauthorized)
HttpNotFound      A response from the device has returned a 404 error code (Not Found)
HttpServerError   A response from the device has returned a 500 error code (Internal Server Error)
HttpTryAgainLater A response from the device has returned a 504 error code (Try again later)
================= ===========

Methods
-------

None.

NetworkError
============

Synopsis
--------

.. code-block:: rust

   #[derive(Debug, Clone)]
   pub struct NetworkError;

Methods
-------

to_http
~~~~~~~

.. code-block:: rust

   pub fn to_http(&self) -> u16

Convert a NetworkErrorKind into an HTTP error code

Examples
--------

.. rubric:: If network send error, convert error to http code and send response

.. code-block:: rust

   #[post("/proxy/<id>/about", format = "json")]
   async fn proxy_route(net: State<'_, AtxNet>, id:String) -> Result<content::Json<String>, Status> {
      match net.get("/ATX/about", &id).await {
         Ok(response) => match response.result {
            Ok(v) => Ok(content::Json(v)),
            Err(e) => Err(Status::new(e.to_http(), "")),
         },
         Err(_) => Err(Status::new(400,"")),
      }
   }
