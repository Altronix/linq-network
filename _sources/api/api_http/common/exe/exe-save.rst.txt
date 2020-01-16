Saving settings to ROM
======================

.. http:post:: /ATX/exe/save

   Save current state into persistent memory of the device. Without saving data, most of the state of the database will not persist across a reboot.

   **Example request**:

   .. sourcecode:: http

      POST /ATX/exe/save HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript
      Content-Type: application/json

      {
         "save":1
      }

   :<json number save: Save profile index. (Currently only supports one profile)

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "error":200
      }
