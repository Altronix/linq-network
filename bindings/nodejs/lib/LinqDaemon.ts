import * as Events from "events";
import { inherits } from "util";
const binding = require("bindings")("linq-network-js");

export interface LinqDaemonConfig {
  http: number;
  https: number;
  zmtp: number;
  zmtps: number;
  cert: string;
  key: string;
  db: string;
}

export class LinqDaemon {
  daemon: any;
  constructor() {
    let self = this;
    self.daemon = new binding.daemon.LinqDaemon();
  }

  start(config: LinqDaemonConfig, ms?: number): LinqDaemon {
    let self = this;
    self.daemon.start(config);
    // TODO add poll and isRunning()
    (function poll() {
      setTimeout(
        () => {
          if (self.daemon.isRunning()) poll();
          self.daemon.poll(0);
        },
        ms ? ms : 50
      );
    })();
    return self;
  }
}

export const daemon = new LinqDaemon();
export default daemon;
