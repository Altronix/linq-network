/api/v1/public/login
====================

.. http:post:: /api/v1/public/login

   Login to the LinQ Service and receive an `API token <https://jwt.io>`_.

   **Example request:**

   .. sourcecode:: http

      POST /api/v1/public/login HTTP/1.0
      Host: 192.168.0.22
      Accept: application/json, text/javascript
      Content-Type: application/json

      {
         "user": "John Doe"
         "pass": "SupER SecReT 123456789"
      }

   **Example response:**

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c"
      }
