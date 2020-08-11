import { API as USER_API } from './user/constants';
export default API;
export const API = {
  DEVICES: 'api/v1/devices',
  ALERTS: 'api/v1/alerts',
  ...USER_API
};
