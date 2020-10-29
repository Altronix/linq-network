import {
  network as linq,
  UpdateDashboard,
  LinqNetwork,
} from "../../../build/install/lib";
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
import * as fs from "fs";

async function readUpdate(path: string) {
  return fs.promises.readFile(path, "utf-8");
}

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

function cloud(cloud: boolean, serial: string, t: number) {
  return timer(t).pipe(
    take(1),
    switchMap(() => {
      return setCloudViaHttp(serial, cloud).pipe(
        mapTo({
          serial,
          step: `Cloud ${cloud ? "enabled" : "disabled"}`,
          response: "",
        })
      );
    })
  );
}

const enableCloud = (serial: string, timer: number) =>
  cloud(true, serial, timer);

const disableCloud = (serial: string, timer: number) =>
  cloud(false, serial, timer);

const update = __dirname + "/linq2-2.4.6-dashboard.json";
let running = linq.run(10);

/**
 * @brief Listen for a device, and send an update. 2 seconds into an update
 *        use the HTTP api to disable the cloud service.  Then 15 seconds
 *        later, use the HTTP api to re-enable the cloud service.  The update
 *        should continue
 */
linq
  .listen(33455)
  .events("new")
  .pipe(
    take(1),
    switchMap(({ serial }) =>
      merge(
        // Send an update
        from(fs.promises.readFile(update, "utf-8")).pipe(
          map((text) => JSON.parse(text) as UpdateDashboard),
          switchMap((update) => linq.update(serial, update.files[1].update)),
          map(({ response, remaining }) => {
            return { step: remaining, response };
          })
        ),

        // Disable cloud via HTTP request x seconds into update
        disableCloud(serial, 2000),

        // Restore cloud x seconds into update
        enableCloud(serial, 15000)
      )
    )
  )
  .subscribe(
    ({ step, response }) => {
      console.log(step);
      console.log(response);
      if (step === 0) linq.shutdown();
    },
    (e) => {
      console.error(e);
      linq.shutdown();
    }
  );
