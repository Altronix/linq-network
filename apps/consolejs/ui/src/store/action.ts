import { actions as router, Actions as Router } from './router/action';
import { actions as devices, Actions as Devices } from './devices/action';

export const actions = { router, devices };
export type RootActions = Router | Devices;
export type RootDispatch = typeof actions;
