import { actions as router, Actions as Router } from './router/action';
import { actions as devices, Actions as Devices } from './devices/action';
import { actions as m5network, Actions as M5Network } from './m5-netw/action';

export const actions = { router, devices, m5network };
export type RootActions = Router | Devices | M5Network;
export type RootDispatch = typeof actions;
