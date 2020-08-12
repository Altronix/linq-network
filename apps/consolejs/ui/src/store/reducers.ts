import { combineReducers } from 'redux';
import routerReducers from './router/reducer';
import devicesReducers from './devices/reducer';
import m5networkReducers from './m5-netw/reducer';

// Combine all reducers
export const rootReducer = combineReducers({
  router: routerReducers,
  devices: devicesReducers,
  m5Network: m5networkReducers
});

export type RootState = ReturnType<typeof rootReducer>;
