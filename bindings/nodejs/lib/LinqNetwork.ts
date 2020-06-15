import * as Events from "events";
import { inherits } from "util";
import { Method, LINQ_EVENTS, AboutData, Binding, Devices } from "./types";
const binding = require("bindings")("linq-network");

export class LinqNetwork extends Events.EventEmitter {
  netw: Binding;
  running: boolean = true;
  shutdownPromise: any;
  private shutdownTimer: any;
  private shutdownResolve: any;
  private _devices: Devices = {};
  get devices() {
    return this._devices;
  }

  constructor(b?: Binding) {
    super();
    let self = this;
    this.netw = b || new binding.network.LinqNetwork();
    this.netw.registerCallback(async function(
      event: LINQ_EVENTS,
      ...args: any[]
    ) {
      switch (event) {
        case "new":
          let serial: string = args[0];
          try {
            let response = await self.send(serial, "GET", "/ATX/about");
            const about = (response.about || response) as AboutData;
            self.devices[serial] = about;
            self.emit(event, { serial, ...about });
          } catch (e) {
            self.emit("error", { serial, error: e });
          }
          break;
        case "heartbeat":
        case "alert":
        case "error":
        case "ctrlc":
        default:
          self.emit(event, ...args);
      }
    });
  }

  version() {
    return this.netw.version();
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

  async send<T>(
    serial: string,
    meth: Method,
    path: string,
    data?: T
  ): Promise<any> {
    const d = data ? JSON.stringify(data) : "";
    let response = await this.netw.send(serial, meth, path, d);
    try {
      return JSON.parse(response);
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

  // deviceCount
  deviceCount(): number {
    return this.netw.deviceCount();
  }

  // nodeCount
  nodeCount(): number {
    return this.netw.nodeCount();
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

  // run
  run(ms: number) {
    let self = this;
    this.shutdownPromise = new Promise(resolve => {
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
