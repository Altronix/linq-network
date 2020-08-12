import { html } from 'lit-element';
import { Device } from './store/devices/state';
import { store, actions } from '../.storybook/dispatcher';

export default { title: 'app' };
// Some dummy data
let devices: Device[] = [];
for (let i = 0; i < 1; i++)
  devices.push({
    serial: '334hgfqREdzsued==',
    siteId: 'foo',
    product: 'LinQ2',
    prjVersion: '2.0.1',
    atxVersion: '2.0.2',
    webVersion: '2.0.0',
    mac: 'CB:45:AD:99:5D',
    lastSeen: Math.floor(new Date().getTime() / 1000)
  });

store.dispatch(actions.devices.fetchOk({ devices }));
export const basic = () => html` <atx-app></atx-app> `.getHTML();
