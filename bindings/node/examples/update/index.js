"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
const __1 = require("../../../../");
const operators_1 = require("rxjs/operators");
const fs = __importStar(require("fs"));
if (!(process.argv.length > 2))
    process.exit(-1);
const update = JSON.parse(fs.readFileSync(process.argv[2], "utf-8"));
// hack
let serial = "";
__1.network
    .tick(10)
    .listen(33455)
    .events("new")
    .pipe(operators_1.filter((ev) => ev.product.toLowerCase() === "linq2"), operators_1.take(1), operators_1.switchMap((ev) => {
    serial = ev.serial;
    console.log(__1.network.devices[serial]);
    return __1.network.update(serial, update);
}))
    .subscribe((message) => console.log(message), (error) => console.error(error), () => {
    console.log(__1.network.devices[serial]);
    __1.network.shutdown();
});
