/**
 * @Brief TlsTerminate class
 *
 * Simple module for prototyping.  Production enviornment options for
 * TLS terminating could be:
 * https://github.com/indutny/bud
 * https://github.com/haproxy/haproxy
 * https://github.com/varnish/hitch
 */

import * as tls from "tls";

let fs = require("fs"),
  net = require("net");

export interface Options {
  cert: string;
  key: string;
  zmtp: number;
  zmtps: number;
}
export namespace TlsTerminate {
  export const listen = (options: Options) => {
    return new Promise((resolve, reject) => {
      let tlsOptions = {
        cert: fs.readFileSync(options.cert),
        key: fs.readFileSync(options.key)
      };
      let netOptions = {
        port: options.zmtp
      };
      let server = tls
        .createServer(tlsOptions, tlsSocket => {
          let tcpSocket = net.connect(netOptions);
          tlsSocket.pipe(tcpSocket);
          tcpSocket.pipe(tlsSocket);
          tlsSocket.on("error", function(e) {
            console.log(e);
            tlsSocket.destroy();
            tcpSocket.end();
          });
          tcpSocket.on("error", function(e: any) {
            console.log(e);
            tcpSocket.destroy();
            tlsSocket.end();
          });
          tlsSocket.on("end", function() {
            tcpSocket.end();
          });
          tcpSocket.on("end", function() {
            tlsSocket.end();
          });
        })
        .listen(options.zmtps, () => resolve(server));
    });
  };
}

