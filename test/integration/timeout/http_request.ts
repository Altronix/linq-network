import { Observable } from "rxjs";
import { switchMap } from "rxjs/operators";
import * as http from "http";

export const httpRequest = <R>(
  dest: string,
  method: "GET" | "POST" | "DELETE",
  path: string,
  data?: object
): Observable<R> =>
  new Observable<R>((subscriber) => {
    const post: string = data ? JSON.stringify(data) : "";
    const [host, port] = dest.split(":");
    const auth = `Basic ${Buffer.from("admin:admin").toString("base64")}`;
    const opts: http.RequestOptions = {
      host,
      port,
      path,
      method,
      headers: {
        ["Content-Length"]: post.length,
        ["Authorization"]: auth,
      },
    };
    const client = http.request(opts);
    const resolve = (response: R) => {
      subscriber.next(response);
      subscriber.complete();
    };
    const error = (r: any) => {
      subscriber.error(r);
      subscriber.complete();
    };
    let response: string = "";
    client.on("error", (e) => error(e));
    client.on("timeout", () => error({ error: "timeout" }));
    client.on("response", (r) => {
      r.on("data", (data) => {
        response += data;
      });
      r.on("end", () => resolve(JSON.parse(response)));
      r.on("error", (e) => error({ error: e }));
    });
    client.write(post);
    client.end();
  });
