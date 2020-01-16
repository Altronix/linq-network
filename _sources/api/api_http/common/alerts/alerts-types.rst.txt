Alert types
===========

.. http:get:: /ATX/alerts/types

   Returns a list of all the supported alert types for the product. Different products may support different alert types.  For a detailed list of each alert type per product please reference the product specific api notes for your specific product. Below describes the API format in general form for all products.

   **Example request**:

   .. sourcecode:: http

      GET /ATX/alerts/types HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
        "types": {
          "acFail": {
            "name": "acFail",
            "mesg": "AC input failure",
            "timer": 0
          },
          "acFailR": {
            "name": "acFailR",
            "mesg": "AC input return",
            "timer": 0
          },
          "battFail": {
            "name": "battFail",
            "mesg": "Battery fail",
            "timer": 0
          },
          "battFailR": {
            "name": "battFailR",
            "mesg": "Battery return",
            "timer": 0
          },
          "sysUp": {
            "name": "sysUp",
            "mesg": "System Startup",
            "timer": 0
          },
          "...continued": {
            "...": ""
          }
        }
      }


   :>json string name: The name of alert.  Is a KEY property to map alerts with a human readable message
   :>json string mesg: The message describing the alert.
   :>json number timer: A number in seconds used to filter alerts. When an alert condition only persists for less than x seconds, the alert will be filtered
