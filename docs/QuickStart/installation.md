# Installation

## Installation

Follow the installation guide for your target language and platform

  - [Windows](/QuickStart/installation.md)

  - [Linux (x64)](/QuickStart/installation.md)

  - [Linux (ARM)](/QuickStart/installation.md)

## Add linq-network Dependency

<!-- tabs:start -->

#### ** C/C++ **

CMakeLists.txt
```cmake
add_executable(server main.c)
target_link_libraries(server linq-netw)
install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
```

#### ** Rust **

```bash
# TODO not published yet
```

#### ** Nodejs **

1. Add runtime enviorment to your package.json
   
``` json
// package.json
{
  // ...
  "cmake-js": {
    "runtime":"node", // or electron or nw
    "runtimeVersion": "10.9.0",
    "arch": "x64",
  }
}
```

2. Install @altronix/linq-network-js

```bash
npm install @altronix/linq-network-js
```

<!-- tabs:end -->

## Initialize Application

<!-- tabs:start -->

#### ** C **

main.c
```c
// Include the Altronix Header
#include "altronix/linq_netw.h"

// Declare your application callbacks
static void on_error(void* ctx, E_LINQ_ERROR e, const char *sid, device_s**d);
static void on_alert(void* ctx, linq_netw_alert_s* alert, linq_netw_email_s* mail);
static void on_heartbeat(void* ctx, const char *serial, device_s** d);
linq_netw_callbacks callbacks = {.err = on_error, .alert = on_alert, .hb = on_heartbeat};

int
main(int argc, char *argv[])
{
  ((void)argc); // Not used
  ((void)argv); // Not used

  int err = -1;
  linq_netw_socket s;

  // Create a linq-network context
  linq_netw_s *network = linq_netw_create(&callbacks, NULL);
  if(!server) return err;

  // Listen for incoming device connections...
  s = linq_netw_listen(server, "tcp://*:33455");

  // Process Socket IO every 5ms
  while (sys_running())
  {
    linq_netw_poll(server, 5);
  }

  // Free linq-network context
  linq_netw_destroy(&server);
}

static void 
on_error(void* ctx, E_LINQ_ERROR e, const char *sid, device_s**d)
{
  // 
}

static void 
on_alert(void* ctx, linq_netw_alert_s* alert, linq_netw_email_s* mail)
{
  //
}

static void 
on_heartbeat(void* ctx, const char *serial, device_s** d)
{
  //
}
```

#### ** C++ **

```cpp
#include "altronix/linq_netw.hpp"

int
main(int argc, char *argv[])
{
  altronix::Linq netw{};

  netw.on_error([this](E_LINQ_ERROR error, const char *serial, const char *err) {
      // Received an error
    }).on_heartbeat([this](const char *serial, Device& device) {
      // Received a heartbeat
    }).on_alert([this](linq_alert_s* alert, linq_email_s* email, Device& device) {
      // Received an alert
    });

  while (true)
  {
    netw.poll(5);
  }
}
```

#### ** Rust **

```rust
TODO
```

#### ** NodeJS **
```nodejs
TODO
```

<!-- tabs:end -->
