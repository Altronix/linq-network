import { M5Network } from './state';
export interface M5NetworkFromServer extends M5Network {}

export function fromServer(m5: M5NetworkFromServer): M5Network {
  return m5;
}
