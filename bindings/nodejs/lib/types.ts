export type Method = "GET" | "POST" | "DELETE";
export type LINQ_EVENTS = "heartbeat" | "alert" | "error" | "new" | "ctrlc";

export interface LinqAboutData {
  siteId: string;
  prjVersion: string;
  productKey: string;
  product: string;
  mqxVersion: string;
  atxVersion: string;
  sslVersion: string;
  webVersion: string;
  mfg: string;
  user: string;
  mac: string;
  sid: string;
}

export interface LinqEventHandler {
  onNew?: (serial: string, data: LinqAboutData) => void;
  onHeartbeat?: (serial: string) => void;
  onAlert?: (serial: string) => void;
  onCtrlc?: () => void;
  onError?: () => void;
}

export interface LinqNetworkConstructorArgs {
  eventHandlers: LinqEventHandler[];
}

export interface LinqBinding {
  version: () => string;
  registerCallback: (arg: (ev: LINQ_EVENTS, ...args: any[]) => void) => void;
  on: (event: LINQ_EVENTS, ...args: any[]) => void;
  isRunning: () => boolean;
  poll: (ms: number) => Promise<any>;
  listen: (arg: string | number) => LinqBinding;
  connect: (arg: string | number) => LinqBinding;
  close: (arg: number) => LinqBinding;
  deviceCount: () => number;
  nodeCount: () => number;
  run: (val: number) => void;
  send: <T>(sid: string, meth: Method, path: string, data?: T) => Promise<any>;
}

export interface LinqNetworkConfig extends LinqNetworkConstructorArgs {}

export interface LinqEventHandlerMetadata {
  target: any;
}

export interface LinqEventMetadata extends LinqEventHandlerMetadata {
  event: LINQ_EVENTS;
}
