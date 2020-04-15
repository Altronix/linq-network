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

export interface LinqEventHandlerMetadata {
  target: any;
}

export interface LinqEventMetadata extends LinqEventHandlerMetadata {
  event: LINQ_EVENTS;
}
