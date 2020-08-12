import { M5Network } from './state';
import { Epic, combineEpics, ActionsObservable } from 'redux-observable';
import { RootEpic } from '../epics';
import { Observable } from 'rxjs';
import { Action } from '../types';
import { actions } from '../action';
import { M5NetworkFromServer, fromServer } from './filters';
import * as Actions from './action';

import { of, from } from 'rxjs';
import {
  map,
  switchMap,
  repeat,
  delay,
  startWith,
  takeUntil,
  catchError,
  filter
} from 'rxjs/operators';

export const fetch$: RootEpic = (action$, state$, { io }): Observable<Action> =>
  action$.pipe(
    filter((e): e is Actions.Fetch => e.type === Actions.FETCH),
    switchMap((action) => {
      let url = `api/v1/proxy/${action.serial}/network`;
      return from(io.get<M5NetworkFromServer>(url)).pipe(
        map((response) =>
          actions.m5network.fetchOk({ data: fromServer(response.json) })
        )
      );
    }),
    catchError((error) => of(actions.m5network.fetchErr()))
  );

export default combineEpics(fetch$);
