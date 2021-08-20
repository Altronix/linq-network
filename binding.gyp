{
  "targets":[
    {
      "target_name":"linq",
      "sources":[
        "./bindings/cpp/netw.hpp",
        "./bindings/node/src/binding.cc",
        "./bindings/node/src/LogJS.cc",
        "./bindings/node/src/LogJS.h",
        "./bindings/node/src/LinqJS.cc",
        "./bindings/node/src/LinqJS.h"
      ],
      "include_dirs":[
        "./bindings/cpp/",
        "./bindings/node/src/",
        "./build-linq/install/include/",
        "./build-linq/install/include/altronix/",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [ "LINQ_STATIC", "ZMQ_STATIC", "CZMQ_STATIC" ],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
      "cflags!":["-fno-exceptions"],
      "cflags_cc!":["-fno-exceptions"],
      "conditions":[
        [ "OS=='linux'", {
          "libraries":[
            "../build-linq/install/lib/liblinqnetwork.a",
            "../build-linq/install/lib/libzmq.a",
            "-luuid",
            "-ludev"
          ],
          "copies": [
            {
              "destination": "./dist/linux-x64",
              "files": [
              ]
            }
          ],
        }],
        [ "OS=='win'", {
          "libraries":[
            "../build-linq/install/lib/linqnetwork.lib",
            "../build-linq/install/lib/libzmq-v142-mt-s-4_3_4.lib",
            "uuid",
	    "ws2_32",
	    "iphlpapi",
	    "wldap32",
	    "Rpcrt4"
	  ],
          "copies": [
            {
              "destination": "./dist/win32-x64",
              "files": [
              ]
            }
          ],
          "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
          "msvs_settings": {
            "VCCLCompilerTool": {
	      "RuntimeLibrary": "0",
              "ExceptionHandling": "1",
              "AdditionalOptions": [ "/EHsc" ],
	    }
          }
        }]
      ]
    }
  ]
}
