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
    case Action.POLL_START:
      return pollStart(state, action);
      break;
    case Action.POLL_STOP:
      return pollStop(state, action);
      break;
    case Action.COUNT:
      return count(state, action);
      break;
    case Action.COUNT_OK:
      return countOk(state, action);
      break;
    case Action.COUNT_ERR:
      return countErr(state, action);
      break;
    default:
      return state;
  }
}

function fetch(state: State, action: Action.Fetch): State {
  let ret = { ...state, loading: true };
  if (action.query) Object.assign(ret, action.query);
  return ret;
}

function fetchOk(state: State, action: Action.FetchOk): State {
  return { ...state, loading: false, devices: action.devices };
}

function fetchErr(state: State, action: Action.FetchErr): State {
  return { ...state, loading: false };
}

function pollStart(state: State, action: Action.PollStart): State {
  return { ...state };
}

function pollStop(state: State, action: Action.PollStop): State {
  return { ...state };
}

function count(state: State, action: Action.Count): State {
  let ret = { ...state };
  return ret;
}

function countOk(state: State, action: Action.CountOk): State {
  return { ...state, count: action.count };
}

function countErr(state: State, action: Action.CountErr): State {
  return { ...state };
}
