export interface IoResponse<T> {
  url: string;
  headers: Headers;
  status: number;
  statusText: string;
  json: T;
}
export interface IoRequester {
  get: <T>(url: string) => Promise<IoResponse<T>>;
  put: <T>(url: string, obj: any) => Promise<IoResponse<T>>;
  post: <T>(url: string, obj: any) => Promise<IoResponse<T>>;
  delete: <T>(url: string) => Promise<IoResponse<T>>;
}

export type Fetch = (url: string, config?: any) => Promise<Response>;
