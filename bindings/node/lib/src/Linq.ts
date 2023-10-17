import * as Events from "events";
import { inherits } from "util";
import { of, from, defer, concat, EMPTY, Observable, Subject } from "rxjs";
import {
  switchMap,
  concatMap,
  map,
  tap,
  catchError,
  filter,
  mergeMap,
  takeUntil,
} from "rxjs/operators";
import { isUpdateDashboard, normalizeUpdateDashboard } from "./update";
import {
  Method,
  LINQ_EVENTS,
  Binding,
  LogData,
  Devices,
  DevicesSummary,
  Update,
  UpdateDashboard,
  UpdateResponse,
  AboutResponse,
  Events as Event,
  EventNew,
  EventHeartbeat,
  EventAbout,
  EventAlert,
  EventError,
  EventCtrlc,
  EventLog,
} from "./types";
import {
  isEventDataNew,
  isEventDataHeartbeat,
  isEventDataAlert,
  isEventDataError,
  mapAboutResponse,
  whenNew,
  whenAbout,
  whenHeartbeat,
  whenAlert,
  whenError,
  whenCtrlc,
  takeWhileRunning,
} from "./event";
const binding = require("./linq.node");

export class Linq extends Events.EventEmitter {
  netw: Binding;
  running: boolean = true;
  shutdownPromise: any;
  events$: Subject<Event> = new Subject<Event>();
  private logger: { open: (fn: (data: LogData) => void) => void };
  private _events$: Subject<Event> = new Subject<Event>();
  private shutdownTimer: any;
  private shutdownResolve: any;
  private _devices: Devices = {};
  get devices() {
    const summaries: DevicesSummary = JSON.parse(this.netw.devices());
    this._devices = [...Object.keys(summaries), ...Object.keys(this._devices)]
      .map((k) => ({ ...this._devices[k], ...summaries[k] }))
      .reduce((acc, cur) => ({ [cur.serial]: cur, ...acc }), {});
    return this._devices;
  }

  constructor(b?: Binding) {
    super();

    let self = this;
    this.logger = new binding.Logger();
    this.logger.open((data) => this.events$.next({ type: "log", ...data }));
    this.netw = b || new binding.Linq();
    this.netw.registerCallback(async function (
      event: LINQ_EVENTS,
      ...args: any[]
    ) {
      // Listen to events from our lower level binding, and emit as observable
      if (event === "new" && isEventDataNew(args[0])) {
        self._events$.next({ type: "_new", serial: args[0] });
      } else if (event === "heartbeat" && isEventDataHeartbeat(args[0])) {
        self._events$.next({ type: "heartbeat", serial: args[0] });
      } else if (event === "alert" && isEventDataAlert(args[0])) {
        self._events$.next({ type: "alert", ...args[0] });
      } else if (event === "error" && isEventDataError(args[0])) {
        self._events$.next({ type: "error", ...args[0] });
      } else if (event === "ctrlc") {
        self._events$.next({ type: "ctrlc", ...args[0] });
      } else {
        // TODO log
      }
    });

    // Our observable passes through all the binding events, and listens for
    // _new events to request about data. When about data response arrives we
    // emit a more decorated "new" event.
    self._events$
      .asObservable()
      .pipe(
        takeWhileRunning(self),
        mergeMap((o) =>
          o.type !== "_new"
            ? of(o)
            : from(
                self.send<AboutResponse>(o.serial, "GET", "/ATX/about")
              ).pipe(
                mapAboutResponse(),
                tap((event) => {
                  const sum: DevicesSummary = JSON.parse(self.netw.devices());
                  const sid = sum[event.sid] || {};
                  self._devices[event.sid] = { ...event, ...sid };
                }),
                catchError((_) => EMPTY)
              )
        )
      )
      .subscribe((ev) => {
        // Subscribe to our observable and expose traditional event emitter api
        self.events$.next(ev);
        self.emit(ev.type, { ...ev });
      });
  }

  logs(): Observable<EventLog> {
    return this.events$.asObservable().pipe(
      filter((e): e is EventLog => e.type === "log"),
      takeUntil(this.shutdownPromise)
    );
  }

  events(ev: "_new"): Observable<EventNew>;
  events(ev: "new"): Observable<EventAbout>;
  events(ev: "heartbeat"): Observable<EventHeartbeat>;
  events(ev: "alert"): Observable<EventAlert>;
  events(ev: "error"): Observable<EventError>;
  events(ev: "ctrlc"): Observable<EventCtrlc>;
  events(): Observable<Event>;
  events(ev?: "_new" | LINQ_EVENTS): Observable<Event> {
    if (ev) {
      switch (ev) {
        case "_new":
          return this.events().pipe(whenNew());
        case "new":
          return this.events().pipe(whenAbout());
        case "heartbeat":
          return this.events().pipe(whenHeartbeat());
        case "alert":
          return this.events().pipe(whenAlert());
        case "error":
          return this.events().pipe(whenError());
        case "ctrlc":
          return this.events().pipe(whenCtrlc());
      }
    } else {
      return this.events$.asObservable().pipe(takeUntil(this.shutdownPromise));
    }
  }

  version() {
    return this.netw.version();
  }

  root(path: string): Linq {
    this.netw.root(path);
    return this;
  }

  // listen
  listen(port: number): Linq;
  listen(port: string): Linq;
  listen(port: string | number): Linq {
    if (typeof port === "number") port = `tcp://*:${port}`;
    this.netw.listen(port);
    return this;
  }

  // connect
  connect(port: number): Linq;
  connect(port: string): Linq;
  connect(port: string | number): Linq {
    if (typeof port === "number") port = `tcp://*:${port}`;
    this.netw.connect(port);
    return this;
  }

  async refresh(serial: string): Promise<AboutResponse["about"]> {
    let { about } = await this.send<AboutResponse>(serial, "GET", "/ATX/about");
    this._devices[serial] = { ...this._devices[serial], ...about };
    return about;
  }

  // close
  close(idx: number) {
    this.netw.close(idx);
    return this;
  }

  async send<R, T = any>(
    serial: string,
    meth: Method,
    path: string,
    data?: T
  ): Promise<R> {
    const d: string = data
      ? (typeof data === "string" && data) || JSON.stringify(data)
      : "";
    let response = await this.netw.send(serial, meth, path, d);
    try {
      return JSON.parse(response) as R;
    } catch (e) {
      throw e;
    }
  }

  // post
  post<T>(serial: string, path: string, data: T) {
    return this.send(serial, "POST", path, data);
  }

  // get
  get(serial: string, path: string) {
    return this.send(serial, "GET", path);
  }

  // delete
  delete(serial: string, path: string) {
    return this.send(serial, "DELETE", path);
  }

  // Alias for deviceCount
  count(): number {
    return this.deviceCount();
  }

  // deviceCount
  deviceCount(): number {
    return this.netw.deviceCount();
  }

  // nodeCount
  nodeCount(): number {
    return this.netw.nodeCount();
  }

  // remove
  remove(sid: string): Linq {
    if (this._devices[sid]) delete this._devices[sid];
    this.netw.deviceRemove(sid);
    return this;
  }

  // Shutdown our run timer
  shutdown(): Linq {
    this.running = false;
    if (this.shutdownTimer) {
      clearTimeout(this.shutdownTimer);
      this.shutdownTimer = undefined;
      this.netw.earlyDestruct();
      this.shutdownResolve();
    }
    return this;
  }

  scan(): number {
    return this.netw.scan();
  }

  update(
    serial: string,
    update: Update[] | UpdateDashboard
  ): Observable<UpdateResponse> {
    type E = { error: number };
    const URL = "/ATX/exe/update";
    const packets = isUpdateDashboard(update)
      ? normalizeUpdateDashboard(update)
      : update;
    return concat(
      of(packets).pipe(
        switchMap((update) => {
          let remaining = update.length;
          return from(update).pipe(
            concatMap((u) => this.send<E>(serial, "POST", URL, u)),
            map((response) => ({ response, remaining: --remaining }))
          );
        })
      ),
      defer(() => this.refresh(serial)).pipe(switchMap((_) => EMPTY))
    );
  }

  tick(ms: number) {
    this.run(ms);
    return this;
  }

  // run
  run(ms: number) {
    let self = this;
    this.shutdownPromise = new Promise((resolve) => {
      self.shutdownResolve = resolve;
      (function poll() {
        self.shutdownTimer = setTimeout(() => {
          self.netw.poll(ms);
          if (self.netw.isRunning() && self.running) {
            poll();
          } else {
            // Arrive here, then we received ctrlc signal or caller told us to
            // shutdown. We use earlyDestruct() because node garbage collector
            // usually doesn't free our binding...
            self.running = false;
            self.netw.earlyDestruct();
            self.shutdownResolve();
          }
        }, ms);
      })();
    });
    return this.shutdownPromise;
  }
}

inherits(Linq, Events.EventEmitter);

export const network = new Linq();
export default network;
