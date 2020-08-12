import { combineReducers } from 'redux';
import routerReducers from './router/reducer';
import devicesReducers from './devices/reducer';

// Combine all reducers
export const rootReducer = combineReducers({
  router: routerReducers,
  devices: devicesReducers
});

export type RootState = ReturnType<typeof rootReducer>;
