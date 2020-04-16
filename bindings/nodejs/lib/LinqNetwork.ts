import * as Events from "events";
import { inherits } from "util";
import { Method, LINQ_EVENTS, LinqAboutData } from "./types";
const binding = require("bindings")("linq-network");

export class LinqNetwork extends Events.EventEmitter {
  netw: any;

  constructor() {
    super();
    let self = this;
    this.netw = new binding.network.LinqNetwork();
    this.netw.registerCallback(async function(
      event: LINQ_EVENTS,
      ...args: any[]
    ) {
      switch (event) {
        case "new":
          let serial: string = args[0];
          try {
            let response = await self.send(serial, "GET", "/ATX/about");
            // TODO validate response.about
            self.emit(event, serial, response.about as LinqAboutData);
          } catch (e) {
            self.emit("error", { serial, ...e });
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
    return this.netw.nodeCound();
  }

  // run
  run(ms: number) {
    let self = this;
    return new Promise(resolve => {
      (function poll() {
        setTimeout(() => {
          self.netw.poll(ms);
          if (self.netw.isRunning()) {
            poll();
          } else {
            resolve();
          }
        }, ms);
      })();
    });
  }
}

inherits(LinqNetwork, Events.EventEmitter);

export const network = new LinqNetwork();
export default network;
