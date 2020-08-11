import { IoRequester, IoResponse, Fetch } from './types';
import { SYMBOLS } from '../../ioc/constants.root';

const GET = 'get';
const PUT = 'put';
const POST = 'post';
const DELETE = 'delete';
type METHOD = typeof GET | typeof PUT | typeof POST | typeof DELETE;

type Headers = {
  [key: string]: string;
};

export class IoService implements IoRequester {
  headers: Headers = {};
  constructor(private fetch: Fetch) {}

  setHeader(key: string, value: string) {
    this.headers[key] = value;
  }

  removeHeader(key: string) {
    delete this.headers[key];
  }

  async request<T>(
    method: METHOD,
    url: string,
    obj?: any
  ): Promise<IoResponse<T>> {
    let config = { credentials: 'same-origin', method };
    let headers = this.headers;
    if (Object.keys(headers).length) Object.assign(config, { headers });
    if (obj) {
      headers['Content-Type'] = 'application/json';
      Object.assign(config, { headers }, { body: JSON.stringify(obj) });
    }
    let response = await this.fetch(url, config);
    await new Promise(resolve => setTimeout(() => resolve(), 300));
    let json = await response.json().catch(e => false);
    let ret = Object.assign({}, response, { json });
    if (!(response.status === 200)) throw ret;

    return ret;
  }

  async get<T>(url: string): Promise<IoResponse<T>> {
    return this.request(GET, url);
  }

  async put<T>(url: string, obj: any): Promise<IoResponse<T>> {
    return this.request(PUT, url, obj);
  }

  async post<T>(url: string, obj: any): Promise<IoResponse<T>> {
    return this.request(POST, url, obj);
  }

  async delete<T>(url: string): Promise<IoResponse<T>> {
    return this.request(DELETE, url);
  }
}
