import * as Action from './action';
import { State, initial } from './state';

export default function reducer(state = initial, action: Action.Actions) {
  switch (action.type) {
    case Action.FETCH:
      return fetch(state, action);
      break;
    case Action.FETCH_OK:
      return fetchOk(state, action);
      break;
    case Action.FETCH_ERR:
      return fetchErr(state, action);
      break;
    default:
      return state;
  }
}

function fetch(state: State, action: Action.Fetch): State {
  return { ...state, loading: true, result: 'PENDING' };
}

function fetchOk(state: State, action: Action.FetchOk): State {
  return { ...state, loading: false, data: action.data, result: 'SUCCESS' };
}

function fetchErr(state: State, action: Action.FetchErr): State {
  return { ...state, loading: false, result: 'ERROR' };
}
