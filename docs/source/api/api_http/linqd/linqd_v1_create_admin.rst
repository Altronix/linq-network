/api/v1/public/create_admin
===========================

.. http:post:: /api/v1/public/create_admin

   Create the admin account. If an admin already exists you will not be authorized to create an additional admin. If you lose your admin credentials to the original account you must factory reset your hardware (or re-install the LinQ service).

   **Example request:**

   .. sourcecode:: http

      POST /api/v1/public/create_admin HTTP/1.0
      Host: 192.168.0.22
      Accept: application/json, text/javascript
      Content-Type: application/json

      {
         "user": "John Doe"
         "pass": "SupER SecReT 123456789"
      }

   :<json string user: Name of the admin account
   :<json string pass: Password of the admin account.

   .. warning:: Password must be larger than 12 charectors!

      If an Admin account has already been created, then you can not create an additional admin.  If you lose your admin credentials to the original admin account, then you must factory reset your hardware (or re-install the LinQ Service).

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "error":200
      }
