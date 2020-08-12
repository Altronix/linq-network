import { Device } from './state';
export interface DeviceFromServer extends Device {
  site_id: string;
  prj_version: string;
  atx_version: string;
  web_version: string;
  last_seen: number;
}

export function fromServer(devices: DeviceFromServer[]): Device[] {
  return devices.map(device => {
    return {
      ...device,
      siteId: device.site_id,
      prjVersion: device.prj_version,
      atxVersion: device.atx_version,
      webVersion: device.web_version,
      lastSeen: device.last_seen
    };
  });
}
