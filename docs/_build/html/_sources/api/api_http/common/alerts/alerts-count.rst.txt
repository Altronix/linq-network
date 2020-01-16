Number of alerts since last power up
====================================

.. http:get:: /ATX/alerts/count

   Returns the number of alerts since last power up.  The number is incremented per each alert as they occur.

   **Example request**:

   .. sourcecode:: http

      GET /ATX/alerts/count HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "count":25
      }


   :>json number count: The number of alert events since last power up
