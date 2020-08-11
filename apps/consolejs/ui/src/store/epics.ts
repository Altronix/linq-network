import { Epic, combineEpics, createEpicMiddleware } from 'redux-observable';
import { Dependencies } from './dependencies';
import { RootActions } from './action';
import { RootState } from './reducers';
import userEpic from './users/epics';
import routerEpic from './router/epics';
import deviceEpic from './devices/epics';
import alertEpic from './alerts/epics';
import envEpic from './env/epics';
import networkEpic from './network/epics';
export type RootEpic = Epic<RootActions, RootActions, RootState, Dependencies>;

// Combine all epic middleware
export const rootEpics = combineEpics(
  userEpic,
  routerEpic,
  deviceEpic,
  alertEpic,
  envEpic,
  networkEpic
);
