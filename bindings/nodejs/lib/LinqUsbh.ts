import * as Events from "events";
import { inherits } from "util";
import { Method, UsbhResponse, BindingUsbh } from "./types";
const binding = require("bindings")("linq");

export class LinqUsbh extends Events.EventEmitter {
  usbh: BindingUsbh;
  constructor(b?: BindingUsbh) {
    super();
    this.usbh = b || binding.usbh ? new binding.usbh.LinqUsbh() : null;
    if (!this.usbh) {
      throw new Error("USBH binding not installed!, please recompile binding!");
    }
  }

  scan(vend: number, prod: number): number {
    let vendHex = parseInt(`0x${vend}`),
      prodHex = parseInt(`0x${prod}`);
    return this.usbh.scan(vendHex, prodHex);
  }

  async send<T = any>(
    sid: string,
    meth: Method,
    path: string,
    data?: T
  ): Promise<UsbhResponse> {
    const d = data ? JSON.stringify(data) : "";
    let ret = await this.usbh.send(sid, meth, path, d);
    try {
      return { code: ret.code, data: JSON.parse(ret.data) };
    } catch (e) {
      console.log(`Bad JSON [${ret.data}]`);
      throw e;
    }
  }
}

inherits(LinqUsbh, Events.EventEmitter);

export const usbh = new LinqUsbh();
export default usbh;
