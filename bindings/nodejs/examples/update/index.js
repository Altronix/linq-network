// let linq = require("../../../../build/install/lib/").network;
let linq = require("@altronix/linq").network;
const { from, of } = require("rxjs");
const { takeWhile, take, switchMap, tap, map } = require("rxjs/operators");
const fs = require("fs");
const update = __dirname + "/linq2-2.4.6-dashboard.json";

let running = linq.run(10);

linq
  .listen(33455)
  .events("new")
  .pipe(
    take(1),
    switchMap((e) =>
      from(fs.promises.readFile(update, "utf-8")).pipe(
        map((update) => JSON.parse(update)),
        switchMap((update) => from(linq.update(e.serial, update)))
      )
    )
  )
  .subscribe((e) => {
    console.log(e);
  });
