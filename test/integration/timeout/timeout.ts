import { network as linq, LinqNetwork } from "../../../build/install/lib";
import { from, of, timer, merge, concat, Observable } from "rxjs";
import {
  catchError,
  delay,
  takeWhile,
  map,
  mapTo,
  take,
  switchMap,
  tap,
} from "rxjs/operators";
import { httpRequest } from "./http_request";

function setCloudViaHttp(serial: string, enable: boolean) {
  return httpRequest(
    serial,
    "127.0.0.1:8080",
    "POST",
    "/ATX/network/zmtp/cloud/portEn",
    {
      portEn: enable ? 1 : 0,
    }
  );
}

function disableCloud(serial: string) {
  return setCloudViaHttp(serial, false);
}

function enableCloud(serial: string) {
  return setCloudViaHttp(serial, true);
}

function getAboutViaZmtp(serial: string) {
  return from(linq.send(serial, "GET", "/ATX/about"));
}

let running = linq.run(10);

linq
  .listen(33455)
  .events("new")
  .pipe(
    take(1),
    switchMap(({ serial }) =>
      merge(
        // Disable cloud via HTTP request
        disableCloud(serial).pipe(
          mapTo({ serial, step: "cloud disabled", response: "" })
        ),
        // Send a GET request for About data, hopefully we receve!
        getAboutViaZmtp(serial).pipe(
          map((response) => {
            return { response, serial, step: "response received" };
          })
        ),
        // Wait x seconds and enable cloud via HTTP request
        timer(15000).pipe(
          take(1),
          switchMap(() => {
            return enableCloud(serial).pipe(
              mapTo({ serial, step: "cloud enabled", response: "" })
            );
          })
        )
      )
    )
  )
  .subscribe(
    ({ step, response }) => {
      let resp = JSON.stringify(response);
      if (resp.length > 15) resp = resp.substring(0, 15) + "...";
      console.log(`step: [${step}]`);
      if (resp.length > 2) console.log(`resp: [${resp}]`);
      if (step == "response received") linq.shutdown();
    },
    (e) => {
      console.error(e);
      linq.shutdown();
    }
  );
