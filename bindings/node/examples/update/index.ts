import { network as linq } from "../../../../";
import { merge, from, of } from "rxjs";
import { take, takeWhile, switchMap, tap, map, filter } from "rxjs/operators";
import * as fs from "fs";

const update = JSON.parse(
  fs.readFileSync(__dirname + "/linq2-2.6.3-dashboard.json", "utf-8")
);

linq.logs().subscribe((l) => console.log(l));

linq
  .tick(10)
  .listen(33455)
  .events("new")
  .pipe(
    filter((ev) => ev.product.toLowerCase() === "linq2"),
    take(1),
    switchMap((ev) => linq.update(ev.serial, update)),
    switchMap(({ remaining: r }) => `[UPDAT] => remaining ${r}\n`)
  )
  .subscribe(
    (message) => process.stdout.write(message),
    (error) => console.error(error),
    async () => await linq.shutdown()
  );
