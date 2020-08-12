export type Method = 'STATIC' | 'DHCP';
export type Result = 'SUCCESS' | 'ERROR' | 'PENDING';
export interface M5Network {
  meth: Method;
  hn: string;
  ip: string;
  sn: string;
  gw: string;
  dns0: string;
  dns1: string;
}

export type State = {
  data: M5Network;
  loading: boolean;
  result: Result;
};

export const initial: State = {
  data: {
    meth: 'STATIC',
    hn: '(Loading...)',
    ip: '(Loading...)',
    sn: '(Loading...)',
    gw: '(Loading...)',
    dns0: '(Loading...)',
    dns1: '(Loading...)'
  },
  loading: false,
  result: 'PENDING'
};
