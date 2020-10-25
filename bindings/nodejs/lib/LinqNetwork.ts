import * as Events from "events";
import { inherits } from "util";
import { of, from, Observable, Subject } from "rxjs";
import {
  switchMap,
  concatMap,
  map,
  tap,
  filter,
  mergeMap,
  merge,
  takeWhile,
} from "rxjs/operators";
import { normalize } from "./update";
import {
  Method,
  LINQ_EVENTS,
  Binding,
  Devices,
  Update,
  UpdateDashboard,
  UpdateTypes,
  UpdateResponse,
  AboutResponse,
  Events as Event,
  EventNew,
  EventHeartbeat,
  EventAbout,
  EventAlert,
  EventError,
  EventCtrlc,
  EventData,
  EventDataAbout,
} from "./types";
import {
  isEventDataNew,
  isEventDataAbout,
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
  request,
  takeWhileRunning,
} from "./event";
const binding = require("bindings")("linq");

export class LinqNetwork extends Events.EventEmitter {
  netw: Binding;
  running: boolean = true;
  shutdownPromise: any;
  events$: Subject<Event> = new Subject<Event>();
  private _events$: Subject<Event> = new Subject<Event>();
  private shutdownTimer: any;
  private shutdownResolve: any;
  private _devices: Devices = {};
  get devices() {
    return this._devices;
  }

  constructor(b?: Binding) {
    super();
    let self = this;
    this.netw = b || new binding.LinqNetwork();
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
                  self.devices[event.sid] = event;
                  self.devices[event.sid].lastSeen = new Date();
                })
              )
        )
      )
      .subscribe((ev) => {
        // Subscribe to our observable and expose traditional event emitter api
        self.events$.next(ev);
        self.emit(ev.type, { ...ev });
      });
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
      return this.events$.asObservable();
    }
  }

  version() {
    return this.netw.version();
  }

  root(path: string): LinqNetwork {
    this.netw.root(path);
    return this;
  }

  // listen
  listen(port: number): LinqNetwork;
  listen(port: string): LinqNetwork;
  listen(port: string | number): LinqNetwork {
    if (typeof port === "number") port = `tcp://*:${port}`;
    this.netw.listen(port);
    return this;
  }

  // connect
  connect(port: number): LinqNetwork;
  connect(port: string): LinqNetwork;
  connect(port: string | number): LinqNetwork {
    if (typeof port === "number") port = `tcp://*:${port}`;
    this.netw.connect(port);
    return this;
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
    const d = data ? JSON.stringify(data) : "";
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
  remove(sid: string): LinqNetwork {
    this.netw.deviceRemove(sid);
    return this;
  }

  // Shutdown our run timer
  shutdown(): LinqNetwork {
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

  update<T>(
    serial: string,
    update: Update[] | UpdateDashboard
  ): Observable<UpdateResponse<T>> {
    return of(update).pipe(
      normalize(),
      concatMap((u) => {
        return from(this.send<T>(serial, "POST", "/ATX/exe/update", u)).pipe(
          map((response) => {
            return { response, remaining: u.remaining };
          })
        );
      })
    );
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

inherits(LinqNetwork, Events.EventEmitter);

export const network = new LinqNetwork();
export default network;
