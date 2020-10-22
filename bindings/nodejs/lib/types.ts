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
  root: (arg: string) => Binding;
  listen: (arg: string | number) => Binding;
  connect: (arg: string | number) => Binding;
  close: (arg: number) => Binding;
  deviceCount: () => number;
  deviceRemove: (sid: string) => number;
  nodeCount: () => number;
  scan: () => number;
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

export type UpdateTypes = "firmware" | "website" | "dashboard";
export interface Update {
  type: "firmware" | "website";
  size: number;
  offset: number;
  payload: string;
  md5: string;
}
export interface UpdateFirmware extends Update {
  type: "firmware";
}
export interface UpdateWebsite extends Update {
  type: "website";
}
export interface UpdateDashboard {
  files: [{ update: Update[] }, { update: Update[] }];
}
