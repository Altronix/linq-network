{
  "targets":[
    {
      "target_name":"linq",
      "sources":[
        "./bindings/cpp/netw.hpp",
        "./bindings/nodejs/src/binding.cc",
        "./bindings/nodejs/src/LogJS.cc",
        "./bindings/nodejs/src/LogJS.h",
        "./bindings/nodejs/src/NetwJS.cc",
        "./bindings/nodejs/src/NetwJS.h"
      ],
      "include_dirs":[
        "./bindings/cpp/",
        "./bindings/nodejs/src/",
        "./build-linq/install/include/",
        "./build-linq/install/include/altronix/",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
      "cflags!":["-fno-exceptions"],
      "cflags_cc!":["-fno-exceptions"],
      "conditions":[
        [ "OS=='linux'", {
          "libraries":[
            "../build-linq/install/lib/liblinqnetwork.a",
            "../build-linq/install/lib/liblinqcommon.a",
            "../build-linq/install/lib/liblinqusbh.a",
            "../build-linq/install/lib/liblinqusb.a",
            "../build-linq/install/lib/libusb-1.0.a",
            "../build-linq/install/lib/libzmq.a",
            "../build-linq/install/lib/libczmq.a",
            "-luuid",
            "-ludev"
          ]
        }],
        [ "OS=='win'", {
            "libraries":[]
        }]
      ]
    }
  ]
}
