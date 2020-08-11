import { Epic, combineEpics, ActionsObservable } from 'redux-observable';
import { RootState } from '../reducers';
import { RootActions } from '../action';
import { Dependencies } from '../dependencies';
import { RouterService } from '../../services/router/router.service';
import { Observable } from 'rxjs';
import { isOfType } from 'typesafe-actions';
import { Action } from '../types';
import { Route, ROUTE } from './action';
import { actions } from './action';

import { from } from 'rxjs';
import { map, switchMap, startWith, filter } from 'rxjs/operators';

export const route$: Epic<RootActions, RootActions, RootState, Dependencies> = (
  action$,
  state$,
  { router }
): Observable<Action> =>
  action$.pipe(
    filter(isOfType<typeof ROUTE>(ROUTE)),
    map(action => {
      const ok = router.route(action.route);
      return actions.routeOk();
    })
  );

export default combineEpics(route$);
