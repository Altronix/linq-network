import { html } from 'lit-element';
import { AtxDeviceList } from './device-list';
import { Device } from '../../store/devices/state';
import { styles } from '../bulma/styles';
import { store, actions } from '../../../.storybook/dispatcher';

// Some dummy data
let devices: Device[] = new Array(8);
for (let i = 0; i < 8; i++)
  devices[i] = {
    serial: '334hgfqREdzsued==',
    siteId: 'foo',
    product: 'LinQ2',
    prjVersion: '2.0.1',
    atxVersion: '2.0.2',
    webVersion: '2.0.0',
    mac: 'CB:45:AD:99:5D',
    lastSeen: Math.floor(new Date().getTime() / 1000)
  };

export default { title: 'atx-device-list' };

export const basic = () => {
  let ret = html`<atx-device-list></atx-device-list>`.getHTML();
  store.dispatch(actions.devices.fetchOk({ devices }));
  return ret;
};
