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
        "./build/install/include/",
        "./build/install/include/altronix/",
	"<!@(node ./scripts/generate_includes.js)",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "defines": [ "LINQ_STATIC", "ZMQ_STATIC", "CZMQ_STATIC" ],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
      "cflags!":["-fno-exceptions"],
      "cflags_cc!":["-fno-exceptions"],
      "conditions":[
        [ "OS=='linux'", {
          "libraries":[
	    "<!@(node ./scripts/generate_libraries.js)",
            # common libraries
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
	    "<!@(node ./scripts/generate_libraries.js)",
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
