export interface Device {
  serial: string;
  siteId: string;
  mac: string;
  product: string;
  prjVersion: string;
  atxVersion: string;
  webVersion: string;
  lastSeen: number;
}

export type State = {
  devices: Device[];
  count: number;
  start: number;
  limit: number;
  sort: keyof Device | undefined;
  search: { [P in keyof Device]?: string };
  order: 'ASC' | 'DESC';
  loading: boolean;
};

export const initial: State = {
  devices: [],
  count: 0,
  start: 0,
  limit: 10,
  sort: 'last_seen' as any,
  search: {},
  order: 'DESC',
  loading: false
};
