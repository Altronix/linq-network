/api/v1/proxy/...
=================

.. http:get:: /api/v1/proxy/...

   Forward a HTTP request to a connected LinQ enabled device.

   **Example request:**

   .. sourcecode:: http

      GET /api/v1/proxy/4B_j7lZYk8UByAjU7vjyDB1Uam17wSzri5QMN11q_G8=/ATX/about HTTP/1.0
      Host: 192.168.0.22
      Accept: application/json, text/javascript
      Content-Type: application/json

   **Example response:**

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

        {
          "siteId": "LinQ2-Office",
          "product": "LINQ2",
          "prjVersion": "2.01.32",
          "mqxVersion": "4.2.0",
          "atxVersion": "2.02.01",
          "mac": "CC:7F:39:86:E4:DE",
          "sid": "4B_j7lZYk8UByAjU7vjyDB1Uam17wSzri5QMN11q_G8=",
          "lastSeen": 1580601600
        }


.. http:post:: /api/v1/proxy/...

   Forward a HTTP request to a connected LinQ enabled device.

   **Example request:**

   .. sourcecode:: http

      POST /api/v1/proxy/4B_j7lZYk8UByAjU7vjyDB1Uam17wSzri5QMN11q_G8=/ATX/about HTTP/1.0
      Host: 192.168.0.22
      Accept: application/json, text/javascript
      Content-Type: application/json

      {
         "siteId": "Update Site Id"
      }

   **Example response:**

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

        {
          "error": 200
        }
