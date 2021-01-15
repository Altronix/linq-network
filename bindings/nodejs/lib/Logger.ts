const binding = require("bindings")("linq");
import { Subject } from "rxjs";
import { LogData } from "./types";

export interface LoggerBinding {
  open: (arg: (data: LogData) => void) => void;
  close: () => void;
}

export class Logger {
  private logger: LoggerBinding;
  private logs$: Subject<LogData> = new Subject<LogData>();
  constructor(b?: LoggerBinding) {
    this.logger = b || new binding.Logger();
    this.logger.open((data) => this.logs$.next(data));
  }

  listen() {
    return this.logs$.asObservable();
  }
}
