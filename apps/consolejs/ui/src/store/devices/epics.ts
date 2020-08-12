import { Device } from './state';
import { Epic, combineEpics, ActionsObservable } from 'redux-observable';
import { RootEpic } from '../epics';
import { Observable } from 'rxjs';
import { Action } from '../types';
import { actions } from '../action';
import { DeviceFromServer, fromServer } from './filters';
import { Query } from './action';
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
      let url = 'api/v1/devices';
      let query = state$.value.devices;
      let keys = Object.keys(query.search) as (keyof Device)[];
      url += `?start=${query.start}`;
      url += `&limit=${query.limit}`;
      if (keys.length) url += `&search=${keys[0]}:${query.search[keys[0]]}`;
      if (query.sort) url += `&sort=${query.sort}`;
      if (query.order) url += `&order=${query.order}`;
      return from(io.get<DeviceFromServer[]>(url)).pipe(
        map((response) =>
          actions.devices.fetchOk({ devices: fromServer(response.json) })
        )
      );
    }),
    catchError((error) => of(actions.devices.fetchErr()))
  );

const query: Query<Device> = {
  sort: 'last_seen' as keyof Device,
  order: 'DESC'
};
export const poll$: RootEpic = (action$, state$): Observable<Action> =>
  action$.pipe(
    filter((e): e is Actions.PollStart => e.type === Actions.POLL_START),
    switchMap((poll) =>
      action$.pipe(
        filter((e): e is Actions.FetchOk => e.type === Actions.FETCH_OK),
        delay(poll.ms || 5000),
        map(() => actions.devices.fetch({ query })),
        repeat(),
        takeUntil(action$.pipe(filter((e) => e.type === Actions.POLL_STOP))),
        startWith(actions.devices.fetch({ query }))
      )
    ),
    catchError((error) => of(actions.devices.fetchErr()))
  );

export const count$: RootEpic = (action$, state$, { io }): Observable<Action> =>
  action$.pipe(
    filter((e): e is Actions.Count => e.type === Actions.COUNT),
    switchMap((response) =>
      from(io.get<{ count: number }>('/api/v1/devices/count')).pipe(
        map((response) => {
          const { count } = response.json;
          return actions.devices.countOk({ count });
        })
      )
    ),
    catchError((error) => of(actions.devices.countErr()))
  );

export default combineEpics(fetch$, poll$, count$);
