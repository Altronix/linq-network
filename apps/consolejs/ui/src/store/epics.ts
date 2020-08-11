import { Epic, combineEpics, createEpicMiddleware } from 'redux-observable';
import { Dependencies } from './dependencies';
import { RootActions } from './action';
import { RootState } from './reducers';
import routerEpic from './router/epics';
export type RootEpic = Epic<RootActions, RootActions, RootState, Dependencies>;

// Combine all epic middleware
export const rootEpics = combineEpics(routerEpic);
