import { network as linq } from "../../../../build/install/lib";
import { from, of } from "rxjs";
import { takeWhile, take, switchMap, tap, map } from "rxjs/operators";
import * as fs from "fs";
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
        switchMap((update) => linq.update(e.serial, update))
      )
    )
  )
  .subscribe((e) => {
    console.log(e);
  });
