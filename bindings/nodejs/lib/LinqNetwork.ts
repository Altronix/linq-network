import * as Events from "events";
import { inherits } from "util";
const binding = require("bindings")("linq-network-js");

export type Method = "GET" | "POST" | "DELETE";

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
  listen(port: string) {
    this.netw.listen(port);
    return this;
  }

  // connect
  connect(port: string) {
    this.netw.connect(port);
  }

  // close
  close(idx: number) {
    this.netw.close(idx);
  }

  // devices
  devices() {
    // TODO
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
    (function poll() {
      setTimeout(() => {
        self.netw.poll(0);
        if (self.netw.isRunning()) poll();
      }, ms);
    })();
  }
}

inherits(LinqNetwork, Events.EventEmitter);

export const network = new LinqNetwork();
export default network;
