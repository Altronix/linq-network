import { combineReducers } from 'redux';
import userReducers from './users/reducer';
import routerReducers from './router/reducer';
import deviceReducers from './devices/reducer';
import alertReducers from './alerts/reducer';
import envReducers from './env/reducer';
import networkReducers from './network/reducer';

// Combine all reducers
export const rootReducer = combineReducers({
  users: userReducers,
  router: routerReducers,
  devices: deviceReducers,
  alerts: alertReducers,
  env: envReducers,
  network: networkReducers
});

export type RootState = ReturnType<typeof rootReducer>;
