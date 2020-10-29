import { LinqNetwork, Method } from "../../../build/install/lib";
import { Observable, from } from "rxjs";
import { switchMap, map } from "rxjs/operators";

interface SerialResponse<Response> {
  serial: string;
  response: Response;
}
export const linqRequest = <Response>(
  linq: LinqNetwork,
  meth: Method,
  path: string,
  data?: object
) => (
  source: Observable<{ serial: string }>
): Observable<SerialResponse<Response>> =>
  source.pipe(
    switchMap((o) => {
      return from(linq.send<Response>(o.serial, meth, path, data)).pipe(
        map((response) => {
          return { response, serial: o.serial };
        })
      );
    })
  );
