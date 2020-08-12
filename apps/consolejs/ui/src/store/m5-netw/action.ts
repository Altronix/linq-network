import { Action, actionCreator } from '../types';
import { M5Network } from './state';

//
// ACTION TYPES
//
export const FETCH = 'm5network/fetch';
export const FETCH_OK = 'm5network/fetch/ok';
export const FETCH_ERR = 'm5network/fetch/err';
export const POLL_START = 'm5network/poll/start';
export const POLL_STOP = 'm5network/poll/stop';
export const COUNT = 'm5network/count';
export const COUNT_OK = 'm5network/count/ok';
export const COUNT_ERR = 'm5network/count/err';

//
// ACTION INTERFACES
//
export interface Fetch extends Action<typeof FETCH> {
  serial: string;
}
export interface FetchOk extends Action<typeof FETCH_OK> {
  data: M5Network;
}
export interface FetchErr extends Action<typeof FETCH_ERR> {}

export type Actions = Fetch | FetchErr | FetchOk;

//
// ACTION CREATORS
//
export const actions = {
  fetch: actionCreator<Fetch>(FETCH),
  fetchOk: actionCreator<FetchOk>(FETCH_OK),
  fetchErr: actionCreator<FetchErr>(FETCH_ERR)
};
