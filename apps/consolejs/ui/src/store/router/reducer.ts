import * as Action from './action';
import { State, initial } from './state';

export default function reducer(state = initial, action: Action.Actions) {
  switch (action.type) {
    case Action.ROUTE:
      return route(state, action);
      break;
    case Action.ROUTE_OK:
      return routeOk(state, action);
      break;
    default:
      return state;
  }
}

function route(state: State, action: Action.Route): State {
  const { route } = action;
  return { ...state, route };
}

function routeOk(state: State, action: Action.RouteOk): State {
  return { ...state };
}
