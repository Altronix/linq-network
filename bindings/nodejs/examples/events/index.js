"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const lib_1 = require("../../../../build/install/lib");
lib_1.network
    .listen("tcp://*:33455")
    .tick(50)
    .watch()
    .subscribe((event) => {
    console.log(event);
    console.log(lib_1.network.devices);
});
