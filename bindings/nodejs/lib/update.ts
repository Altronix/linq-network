import { Update, UpdateDashboard } from "./types";

// Make sure property has a key
function hasProp(a: any, key: string): boolean {
  return a.hasOwnProperty(key);
}

// Make sure a JSON object has the required fields to be an "update"
// TODO validate md5?
export function isUpdate(d: any): d is Update {
  // prettier-ignore
  if (
    hasProp(d, "type") && (d.type === "firmware" || d.type === "website") &&
    hasProp(d, "size") &&  typeof d.size === "number" &&
    hasProp(d, "offset") && typeof d.offset === "number" &&
    hasProp(d, "payload") && typeof d.payload === "string" &&
    hasProp(d, "md5") && typeof d.md5 === "string"
  ) {
    return true;
  }
  return false;
}

// Check a Dashboard update has all the right properties
export function isUpdateDashboard(update: any): update is UpdateDashboard {
  return (
    update.files &&
    Array.isArray(update.files) &&
    update.files.filter((f: any) => {
      return (
        f.update &&
        Array.isArray(f.update) &&
        f.update.length &&
        f.update.length === f.update.filter((u: any) => isUpdate(u)).length
      );
    }).length === 2
  );
}

// Take a dashboard update and reduce the fields into a single array
export function normalizeUpdateDashboard(update: UpdateDashboard): Update[] {
  let ret: Update[] = [];
  update.files.forEach((f) => (ret = [...ret, ...f.update]));
  return ret;
}
