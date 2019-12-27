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

  listen(port: string) {
    this.netw.listen(port);
  }

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
