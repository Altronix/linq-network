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
type AtxResponse = { error: number };

const update = __dirname + "/linq2-2.4.6-dashboard.json";

// Read a JSON update from file system and send the update to device
function sendUpdate(
  path: string,
  serial: string
): Observable<{ step: number; response: AtxResponse }> {
  return from(fs.promises.readFile(update, "utf-8")).pipe(
    map((text) => JSON.parse(text) as UpdateDashboard),
    switchMap((update) =>
      linq.update<AtxResponse>(serial, update.files[1].update)
    ),
    map(({ response, remaining }) => {
      return { step: remaining, response };
    })
  );
}

// Use HTTP api to enable/disable cloud of LinQ device
function setCloudViaHttp(enable: boolean) {
  return httpRequest<AtxResponse>(
    "127.0.0.1:8080",
    "POST",
    "/ATX/network/zmtp/cloud/portEn",
    {
      portEn: enable ? 1 : 0,
    }
  );
}

// Wait t milliseconds and then enable / disable the cloud
function cloud(
  cloud: boolean,
  t: number
): Observable<{ response: AtxResponse; step: string }> {
  return timer(t).pipe(
    take(1),
    switchMap(() => {
      return setCloudViaHttp(cloud).pipe(
        map((response) => {
          return { response, step: `Cloud ${cloud ? "enabled" : "disabled"}` };
        })
      );
    })
  );
}

// Small wrapper around cloudEnable/Disable
const enableCloud = (timer: number) => cloud(true, timer);

// Small wrapper around cloudEnable/Disable
const disableCloud = (timer: number) => cloud(false, timer);

/**
 * @brief Listen for a device, and send an update. 2 seconds into an update
 *        use the HTTP api to disable the cloud service.  Then 15 seconds
 *        later, use the HTTP api to re-enable the cloud service.  The update
 *        should continue
 */
let running = linq.run(10);
linq
  .listen(33455)
  .events("new")
  .pipe(
    take(1),
    switchMap(({ serial }) =>
      merge(
        // Send an update
        sendUpdate(update, serial),

        // 2 seconds later disable cloud via HTTP request
        disableCloud(2000),

        // 15 seconds later kestore cloud via HTTP request
        enableCloud(15000)
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
