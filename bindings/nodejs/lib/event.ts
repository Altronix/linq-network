import { Observable, of, from } from "rxjs";
import { switchMap, filter, takeWhile, map } from "rxjs/operators";
import {
  LINQ_EVENTS,
  AboutResponse,
  Events,
  EventNew,
  EventAbout,
  EventHeartbeat,
  EventAlert,
  EventError,
  EventCtrlc,
  EventDataNew,
  EventDataHeartbeat,
  EventDataAbout,
  EventDataAlert,
  EventDataError,
  Method,
} from "./types";

type sender = <R, T = any>(
  serial: string,
  meth: Method,
  path: string,
  data?: T
) => Promise<R>;

export function isEventDataNew(data: Object): data is EventDataNew {
  return typeof data === "string";
}

export function isEventDataAbout(data: Object): data is EventDataAbout {
  return (
    data.hasOwnProperty("serial") &&
    data.hasOwnProperty("product") &&
    data.hasOwnProperty("prjVersion") &&
    data.hasOwnProperty("atxVersion") &&
    data.hasOwnProperty("siteId")
  );
}

export function isEventDataHeartbeat(data: Object): data is EventDataHeartbeat {
  return typeof data === "string";
}

export function isEventDataAlert(data: Object): data is EventDataAlert {
  return (
    data.hasOwnProperty("who") &&
    data.hasOwnProperty("what") &&
    data.hasOwnProperty("where") &&
    data.hasOwnProperty("when") &&
    data.hasOwnProperty("mesg") &&
    data.hasOwnProperty("serial")
  );
}

export function isEventDataError(data: Object): data is EventDataError {
  return (
    data.hasOwnProperty("serial") &&
    data.hasOwnProperty("errorCode") &&
    data.hasOwnProperty("errorMessage")
  );
}

export const whenEvent = <T extends Events>(key: LINQ_EVENTS | "_new") => (
  source: Observable<Events>
) => source.pipe(filter((e): e is T => e.type === key));

export const whenNew = () => (source: Observable<Events>) =>
  source.pipe(whenEvent<EventNew>("_new"));

export const whenAbout = () => (source: Observable<Events>) =>
  source.pipe(whenEvent<EventAbout>("new"));

export const whenHeartbeat = () => (source: Observable<Events>) =>
  source.pipe(whenEvent<EventHeartbeat>("heartbeat"));

export const whenAlert = () => (source: Observable<Events>) =>
  source.pipe(whenEvent<EventAlert>("alert"));

export const whenError = () => (source: Observable<Events>) =>
  source.pipe(whenEvent<EventError>("error"));

export const whenCtrlc = () => (source: Observable<Events>) =>
  source.pipe(whenEvent<EventCtrlc>("ctrlc"));

export const request = <T>(
  s: sender,
  meth: Method,
  path: string,
  data?: any
) => (source: Observable<{ serial: string }>) =>
  source.pipe(switchMap((o) => s<T>(o.serial, meth, path, data)));

export const mapAboutResponse = () => (
  source: Observable<AboutResponse>
): Observable<EventAbout> =>
  source.pipe(
    map((response) => {
      return { type: "new", serial: response.about.sid, ...response.about };
    })
  );

export const takeWhileRunning = (running: { running: boolean }) => (
  source: Observable<Events>
) => source.pipe(takeWhile((o) => o.type !== "ctrlc" && running.running));
