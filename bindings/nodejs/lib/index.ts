import * as Events from "events";
const binding = require("bindings")("linq-netw-js");
const emitter = new Events.EventEmitter();

export class LinqNetwork {
  netw: any;
  constructor() {
    this.netw = new binding.LinqNetwork(emitter.emit.bind(emitter));
  }

  listen(port: string) {
    this.netw.listen(port);
  }
}

export default LinqNetwork;
