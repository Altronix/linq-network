let linq = require("../../../../build/install/lib/").network;
const { from, of } = require("rxjs");
const { takeWhile, take, switchMap } = require("rxjs/operators");
const fs = require("fs");

let running = linq.run(10);

linq
  .listen(33455)
  .events()
  .pipe(
    take(1),
    switchMap((e) => of(e))
  )
  .subscribe((e) => {
    console.log(e);
    linq.shutdown();
  });
linq.on("new", (data) => console.log(data));
