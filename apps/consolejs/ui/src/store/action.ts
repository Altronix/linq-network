import { actions as router, Actions as Router } from './router/action';

export const actions = { router };
export type RootActions = Router;
export type RootDispatch = typeof actions;
