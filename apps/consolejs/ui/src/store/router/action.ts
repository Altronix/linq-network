import { Action, actionCreator } from '../types';

//
// ACTION TYPES
//
export const ROUTE = 'router/route';
export const ROUTE_OK = 'router/route/ok';

//
// ACTION INTERFACES
//
export interface Route extends Action<typeof ROUTE> {
  route: string;
}
export interface RouteOk extends Action<typeof ROUTE_OK> {}
export type Actions = Route | RouteOk;

// ACTION CREATORS
export const actions = {
  route: actionCreator<Route>(ROUTE),
  routeOk: actionCreator<RouteOk>(ROUTE_OK)
};
