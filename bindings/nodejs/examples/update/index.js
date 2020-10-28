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
    if (mod != null) for (var k in mod) if (k !== "default" && Object.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
Object.defineProperty(exports, "__esModule", { value: true });
const update = __dirname + "/linq2-2.4.6-dashboard.json";
const lib_1 = require("../../../../build/install/lib");
const rxjs_1 = require("rxjs");
const operators_1 = require("rxjs/operators");
const fs = __importStar(require("fs"));
let running = lib_1.network.run(10);
lib_1.network
    .listen(33455)
    .events("new")
    .pipe(operators_1.take(1), operators_1.switchMap((e) => rxjs_1.from(fs.promises.readFile(update, "utf-8")).pipe(operators_1.map((update) => JSON.parse(update)), operators_1.switchMap((update) => lib_1.network.update(e.serial, update)))))
    .subscribe((e) => {
    console.log(e);
});
