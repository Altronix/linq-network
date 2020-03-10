import * as Events from "events";
import { inherits } from "util";
import { TlsTerminate } from "./tlsTerminate";
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
  tls: any;
  constructor() {
    let self = this;
    self.daemon = new binding.daemon.LinqDaemon();
  }

  start(config: LinqDaemonConfig, ms?: number): Promise<any> {
    let self = this;
    return config.zmtps
      ? TlsTerminate.listen(config).then((tls: any) => {
          self.tls = tls;
          _start();
        })
      : new Promise((resolve, _) => resolve(_start()));

    function _start() {
      self.daemon.start(config);
      (function poll() {
        setTimeout(
          () => {
            if (self.daemon.isRunning()) {
              poll();
            } else {
              self.tls.close();
            }
            self.daemon.poll(0);
          },
          ms ? ms : 50
        );
      })();
      return self;
    }
  }
}

export const daemon = new LinqDaemon();
export default daemon;
