"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const __1 = require("../../../../");
__1.network
    .listen("tcp://*:33455")
    .tick(50)
    .events()
    .subscribe((event) => {
    console.log(event);
});
