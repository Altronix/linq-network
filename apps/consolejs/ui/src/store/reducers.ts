import { combineReducers } from 'redux';
import routerReducers from './router/reducer';

// Combine all reducers
export const rootReducer = combineReducers({
  router: routerReducers
});

export type RootState = ReturnType<typeof rootReducer>;
