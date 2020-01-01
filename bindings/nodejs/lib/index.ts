import * as Events from "events";
import { inherits } from "util";
const binding = require("bindings")("linq-netw-js");

export class LinqNetwork extends Events.EventEmitter {
  netw: any;

  constructor() {
    super();
    let self = this;
    this.netw = new binding.LinqNetwork();
    this.netw.registerCallback(function(event: string, serial: string) {
      self.emit(event, serial);
    });
  }

  // listen
  listen(port: string) {
    this.netw.listen(port);
  }

  // connect
  connect(port: string) {
    this.netw.connect(port);
  }

  // closeRouter
  closeRouter(idx: number) {
    this.netw.closeRouter(idx);
  }

  // closeDealer
  closeDealer(idx: number) {
    this.netw.closeDealer(idx);
  }

  // closeHttp
  closeHttp(idx: number) {
    this.netw.closeHttp(idx);
  }

  // devices
  devices() {
    // TODO
  }

  // get
  sendGet(serial: string, path: string): Promise<any> {
    return this.netw.sendGet(serial, path);
  }

  // post
  post() {
    // TODO
  }

  // delete
  delete() {
    // TODO
  }

  // send
  send() {
    // TODO
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

export default LinqNetwork;
