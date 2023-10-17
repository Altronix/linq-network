import { network as linq } from "../../../../";
import { take, takeWhile, switchMap, tap, map, filter } from "rxjs/operators";
import * as fs from "fs";

if (!(process.argv.length > 2)) process.exit(-1);

const update = JSON.parse(fs.readFileSync(process.argv[2], "utf-8"));

// hack
let serial = "";

linq
  .tick(10)
  .listen(33455)
  .events("new")
  .pipe(
    filter((ev) => ev.product.toLowerCase() === "linq2"),
    take(1),
    switchMap((ev) => {
      serial = ev.serial;
      console.log(linq.devices[serial]);
      return linq.update(serial, update);
    })
  )
  .subscribe(
    (message) => console.log(message),
    (error) => console.error(error),
    () => {
      console.log(linq.devices[serial]);
      linq.shutdown();
    }
  );
