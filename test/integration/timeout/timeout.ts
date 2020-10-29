import { network as linq, LinqNetwork } from "../../../build/install/lib";
import { from, of, concat, Observable } from "rxjs";
import {
  catchError,
  delay,
  takeWhile,
  take,
  switchMap,
  tap,
} from "rxjs/operators";
import { httpRequest } from "./http_request";
import { linqRequest } from "./linq_request";

/**
 * @brief TODO figure out why turning portEn off will cause linq-common hang
 */
let running = linq.run(10);
linq
  .listen(33455)
  .events("new")
  .pipe(
    take(1),
    /*
    linqRequest(linq, "POST", "/ATX/network/zmtp/cloud/portEn", { portEn: 0 }),
    */
    linqRequest(linq, "GET", "/ATX/about"),
    delay(5000),
    httpRequest("127.0.0.1", 8080, "/ATX/network/zmtp/cloud/portEn", {
      portEn: 1,
    }),
    catchError((e: number) => of({ error: e }))
  )
  .subscribe((e) => {
    linq.shutdown();
  });
