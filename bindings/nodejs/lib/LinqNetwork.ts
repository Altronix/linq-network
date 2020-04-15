import * as Events from "events";
import { inherits } from "util";
import { Method } from "./types";
const binding = require("bindings")("linq-network");

export class LinqNetwork extends Events.EventEmitter {
  netw: any;

  constructor() {
    super();
    let self = this;
    this.netw = new binding.network.LinqNetwork();
    this.netw.registerCallback(function(event: string, serial: string) {
      self.emit(event, serial);
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

  send<T>(serial: string, meth: Method, path: string, data?: T): Promise<any> {
    const d = data ? JSON.stringify(data) : "";
    return this.netw.send(serial, meth, path, d);
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
