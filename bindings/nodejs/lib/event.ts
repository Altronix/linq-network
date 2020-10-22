import { Observable, of, from } from "rxjs";
import { switchMap, filter, takeWhile } from "rxjs/operators";
import {
  LINQ_EVENTS,
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

export function isEventDataNew(data: any): data is EventDataNew {
  return false;
}

export function isEventDataAbout(data: any): data is EventDataAbout {
  return false;
}

export function isEventDataHeartbeat(data: any): data is EventDataHeartbeat {
  return false;
}

export function isEventDataAlert(data: any): data is EventDataAlert {
  return false;
}

export function isEventDataError(data: any): data is EventDataError {
  return false;
}

export const events = <T extends Events>(key: LINQ_EVENTS | "_new") => (
  source: Observable<Events>
) => source.pipe(filter((e): e is T => e.type === key));

export const whenNew = () => (source: Observable<Events>) =>
  source.pipe(events<EventNew>("_new"));

export const whenAbout = () => (source: Observable<Events>) =>
  source.pipe(events<EventAbout>("new"));

export const whenHeartbeat = () => (source: Observable<Events>) =>
  source.pipe(events<EventHeartbeat>("heartbeat"));

export const whenAlert = () => (source: Observable<Events>) =>
  source.pipe(events<EventAlert>("alert"));

export const whenError = () => (source: Observable<Events>) =>
  source.pipe(events<EventError>("error"));

export const whenCtrlc = () => (source: Observable<Events>) =>
  source.pipe(events<EventError>("ctrlc"));

export const request = <T>(
  s: sender,
  meth: Method,
  path: string,
  data?: any
) => (source: Observable<{ serial: string }>) =>
  source.pipe(switchMap((o) => s<T>(o.serial, meth, path, data)));

export const takeWhileRunning = (running: { running: boolean }) => (
  source: Observable<Events>
) => source.pipe(takeWhile((o) => o.type !== "ctrlc" && running.running));
