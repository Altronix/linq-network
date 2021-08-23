"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const dist_1 = require("../../../../dist");
dist_1.network
    .listen("tcp://*:33455")
    .tick(50)
    .events()
    .subscribe((event) => {
    console.log(event);
});
