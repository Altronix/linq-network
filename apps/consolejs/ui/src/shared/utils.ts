export type DeepPartial<T> = { [P in keyof T]?: DeepPartial<T[P]> };
export type MapOfNumbers = { [x: string]: number };
export type MapOfBooleans = { [x: string]: boolean };
export type ErrorResponse = { error: number };
export type Omit<T, K extends keyof T> = Pick<T, Exclude<keyof T, K>>;
export type PropType<TObj, TProp extends keyof TObj> = TObj[TProp];

export function bitmapToArray(mask: number, sz: number): number[] {
  let results: number[] = [];
  let arr = mask.toString(2).split("");
  arr.forEach(function (elem: string) {
    results.push(parseInt(elem));
  });
  for (let i = results.length; i < sz; i++) results.unshift(0);
  return results;
}

export function keys<O>(o: O) {
  return Object.keys(o) as (keyof O)[];
}

export function arrayToBitmap(arr: number[]): number {
  return parseInt(arr.join(""), 2);
}

export function diff<T extends {}>(a: T, b: T): DeepPartial<T> {
  let ret: DeepPartial<T> = {};
  for (let key in a) {
    let prop = a[key];
    let t = typeof prop;
    if (t === "string" || t === "number" || t === "boolean") {
      if (a[key] !== b[key]) ret = Object.assign({}, ret, { [key]: a[key] });
    } else if (t === "object") {
      ret[key] = diff(a[key], b[key]);
    }
  }
  return ret;
}

export function isDiff<T extends {}>(a: T, b: T): boolean {
  return !!Object.keys(diff(a, b)).length;
}

export function convertBooleansToNumbers<
  T extends {},
  K extends keyof T & keyof MapOfNumbers
>(data: Partial<T>, keys: K[]): MapOfNumbers {
  let ret: MapOfNumbers = {};
  keys.forEach((key) => {
    let prop: any | undefined = data[key];
    if (typeof prop == "boolean") ret[key] = boolToNum(prop);
  });
  return ret;
}

export function convertNumbersToBooleans<
  T extends {},
  K extends keyof T & keyof MapOfBooleans
>(data: Partial<T>, keys: K[]): MapOfBooleans {
  let ret: MapOfBooleans = {};
  keys.forEach((key) => {
    let prop: any | undefined = data[key];
    if (typeof prop == "number") ret[key] = numToBool(prop);
  });
  return ret;
}

export function convertUxToServer<
  T extends {},
  K extends keyof T & keyof MapOfNumbers
>(data: Partial<T>, keys: K[]): MapOfNumbers {
  let ret: MapOfNumbers = {};
  keys.forEach((key) => {
    let prop: any | undefined = data[key];
    if (typeof prop == "number") ret[key] = scaleServer(prop);
  });
  return ret;
}

export function convertServerToUx<
  T extends {},
  K extends keyof T & keyof MapOfNumbers
>(data: Partial<T>, keys: K[]): MapOfNumbers {
  let ret: MapOfNumbers = {};
  keys.forEach((key) => {
    let prop: any | undefined = data[key];
    if (typeof prop == "number") ret[key] = scaleUx(prop);
  });
  return ret;
}

export function numToBool(data: number): boolean {
  return data == 1 ? true : false;
}

export function boolToNum(data: boolean): number {
  return data == true ? 1 : 0;
}

export function scaleUx(data: number, precision: number = 2) {
  return parseFloat((data / 1000).toFixed(precision));
}

export function scaleServer(data: number) {
  return data * 1000;
}

export function selectMap<T>(arg: T, map: T[]): number;
export function selectMap<T>(arg: number, map: T[]): T;
export function selectMap<T>(arg: number | T, map: T[]) {
  return typeof arg == "number" ? map[arg] : map.indexOf(arg);
}
