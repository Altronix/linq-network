// let linq = require("../../../../build/install/lib/").network;
let linq = require("@altronix/linq").network;
const { from, of } = require("rxjs");
const { takeWhile, take, switchMap } = require("rxjs/operators");
const fs = require("fs");

let running = linq.run(10);

linq.listen(33455).on("new", () => console.log("NEW"));
// .pipe(
//   take(1),
//   switchMap((e) => of(e))
// )
// .subscribe((e) => {
//   console.log(`received event ${JSON.stringify(e)}`);
//   linq.shutdown();
// });
