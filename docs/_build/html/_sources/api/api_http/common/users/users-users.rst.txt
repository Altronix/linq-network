User Management
===============

.. http:get:: /ATX/userManagement/users

   Returns an array of users permissioned to access the device at various restriction levels

   **Example request**:

   .. sourcecode:: http

      GET /ATX/userManagement/users HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "users": ["admin", "thomas"]
      }

.. http:get:: /ATX/userManagement/users/(string:user_id)

   Returned detailed meta-data for a specific user

   **Example request**:

   .. sourcecode:: http

      GET /ATX/userManagement/users/thomas HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "thomas": {
           "user": "thomas",
           "fail": 0,
           "enable": 1,
           "realms": ["/ATX/userManagement/users/thomas/pass", "1:/ATX/userManagement/users", "3:/ATX/exe/save"]
         }
      }

   :>json string user: The name of the user in reference
   :>json string fail: The number of failed login attempts
   :>json number enable: 1 - user is enabled, 0 - user is not enabled
   :>json object realms: An array of URL endpoints that this user is allowed to have access too.


.. http:post:: /ATX/userManagement/users

   Create an additional user

   **Example request**:

   .. sourcecode:: http

      POST /ATX/userManagement/users HTTP/1.0
      Host: 192.168.168.168
      Accept: application/json, text/javascript
      Content-Type: application/json

      {
         "user":"new user name",
         "pass":"new user password",
         "realms":["/"]
      }

   :<json string user: The name of the new user
   :<json string pass: The password of the new user
   :<json number realms: An array of realms this new user shall have access too

   **Example response**:

   .. sourcecode:: http

      HTTP/1.0 200 OK
      Content-Type: application/json

      {
         "error":200
      }
