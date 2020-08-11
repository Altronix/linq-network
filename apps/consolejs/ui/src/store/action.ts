import { actions as user, Actions as User } from './users/action';
import { actions as router, Actions as Router } from './router/action';
import { actions as device, Actions as Device } from './devices/action';
import { actions as alert, Actions as Alert } from './alerts/action';
import { actions as env, Actions as Env } from './env/action';
import { actions as network, Actions as Network } from './network/action';

export const actions = { user, router, device, alert, env, network };
export type RootActions = User | Router | Device | Alert | Env | Network;
export type RootDispatch = typeof actions;
