export type Method = "GET" | "POST" | "DELETE";
export type LINQ_EVENTS = "heartbeat" | "alert" | "error" | "new" | "ctrlc";

export interface AboutData {
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

export interface Devices {
  [x: string]: AboutData & { lastSeen?: Date };
}

export interface Binding {
  version: () => string;
  registerCallback: (arg: (ev: LINQ_EVENTS, ...args: any[]) => void) => void;
  on: (event: LINQ_EVENTS, fn: (...args: any[]) => void) => void;
  isRunning: () => boolean;
  poll: (ms: number) => Promise<any>;
  listen: (arg: string | number) => Binding;
  connect: (arg: string | number) => Binding;
  close: (arg: number) => Binding;
  deviceCount: () => number;
  nodeCount: () => number;
  run: (val: number) => void;
  earlyDestruct: () => void;
  send: <T>(sid: string, meth: Method, path: string, data?: T) => Promise<any>;
}

export interface BindingUsbh {
  version: () => string;
  scan: (vend: number, prod: number) => number;
  send: <T>(
    sid: string,
    meth: Method,
    path: string,
    data?: string
  ) => Promise<UsbhResponseBinding>;
}

export interface UsbhResponse<T = any> {
  code: number;
  data: T;
}
export type UsbhResponseBinding = Omit<UsbhResponse, "data"> & { data: string };
