import { Observable } from "rxjs";
import { switchMap } from "rxjs/operators";
import * as http from "http";

interface SerialResponse<Response> {
  serial: string;
  response: Response;
}
export const httpRequest = (
  host: string,
  port: number,
  path: string,
  data?: object
) => (
  source: Observable<{ serial: string }>
): Observable<SerialResponse<http.IncomingMessage>> =>
  source.pipe(
    switchMap(
      (o) =>
        new Observable<SerialResponse<http.IncomingMessage>>((subscriber) => {
          let post: string = data ? JSON.stringify(data) : "";
          let opts: http.RequestOptions = {
            host,
            port,
            path,
            headers: { ["Content-Length"]: post.length },
          };
          let client = http.request(opts);
          const resolve = (response: http.IncomingMessage) => {
            subscriber.next({ response, serial: o.serial });
            subscriber.complete();
          };
          const error = (r: any) => {
            subscriber.error(r);
            subscriber.complete();
          };
          client.on("error", (e) => error(e));
          client.on("timeout", () => error({ error: "timeout" }));
          client.on("response", (r) => resolve(r));
          client.write(post);
          client.end();
        })
    )
  );
