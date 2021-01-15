export type Method = "GET" | "POST" | "DELETE";
export type LINQ_EVENTS = "heartbeat" | "alert" | "error" | "new" | "ctrlc";
export type LINQ_EVENTS_DATA = string | EventDataAlert | EventDataError;

export interface Devices {
  [x: string]: EventDataAbout & { lastSeen?: Date };
}

export interface Binding {
  version: () => string;
  registerCallback: (arg: (ev: LINQ_EVENTS, ...args: any[]) => void) => void;
  registerLogger: (arg: (log: LogData) => void) => void;
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

export interface LogData {
  tick: number;
  line: number;
  level: string;
  file: string;
  mesg: string;
}

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

export interface UpdateResponse<T = any> {
  response: T;
  remaining: number;
}

export type AboutResponse = { about: Omit<EventDataAbout, "serial"> };
export type EventDataNew = string;
export type EventDataHeartbeat = string;
export interface EventDataAbout {
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
  serial: string;
}

export interface EventDataAlert {
  who: string;
  what: string;
  where: string;
  when: number;
  mesg: string;
  serial: string;
  from: string;
  subject: string;
  user: string;
  password: string;
  server: string;
  port: number;
  to: string[];
}

export interface EventDataError {
  serial: string;
  errorCode: number;
  errorMessage: string;
}

export interface EventNew {
  type: "_new";
  serial: string;
}

export interface EventAbout extends EventDataAbout {
  type: "new";
}

export interface EventHeartbeat {
  type: "heartbeat";
  serial: string;
}

export interface EventAlert extends EventDataAlert {
  type: "alert";
}

export interface EventError extends EventDataError {
  type: "error";
}

export interface EventCtrlc {
  type: "ctrlc";
}

export type Events =
  | EventNew
  | EventAbout
  | EventHeartbeat
  | EventAlert
  | EventError
  | EventCtrlc;

export type EventData<Event> = Omit<Event, "type">;
