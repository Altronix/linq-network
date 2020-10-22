import { Update, UpdateDashboard } from "./types";

function hasProp(a: any, key: string): boolean {
  return a.hasOwnProperty(key);
}

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

export function isUpdateDashboard(update: any): update is UpdateDashboard {
  if (update.files && Array.isArray(update.files)) {
    for (let f in update.files) {
      if (update.files[f].update && Array.isArray(update.files[f].update)) {
        for (let u in update.files[f].update) {
          if (!isUpdate(update.files[f].update[u])) return false;
        }
        return true;
      }
      return false;
    }
    return false;
  }
  return false;
}
