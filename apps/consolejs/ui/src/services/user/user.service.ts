import { IoService } from '../io/io.service';
import { User, UserAdd } from './types';
import { API } from './constants';

interface TokenResponse {
  user: User;
  accessToken: string;
}

export class UserService {
  constructor(private io: IoService) {}

  async refresh() {
    let resp = await this.io.get<TokenResponse>(API.REFRESH);
    this.io.setHeader('Authorization', `Bearer ${resp.json.accessToken}`);
    return resp.json.user;
  }

  async login(email: string, password: string): Promise<User> {
    let resp = await this.io.post<TokenResponse>(API.LOGIN, {
      email,
      password
    });
    this.io.setHeader('Authorization', `Bearer ${resp.json.accessToken}`);
    return resp.json.user;
  }

  async logout(): Promise<void> {
    await this.io.get<string>(API.LOGOUT);
    this.io.removeHeader('Authorization');
  }

  async get(): Promise<User[]> {
    let resp = await this.io.get<User[]>(API.USERS);
    return resp.json;
  }

  async create(u: UserAdd): Promise<{ message: string }> {
    let resp = await this.io.post<{ message: string }>(API.USERS, u);
    return resp.json;
  }

  async remove(email: string): Promise<{ message: string }> {
    const url = `${API.USERS}?email=${email}`;
    let resp = await this.io.delete<{ message: string }>(url);
    return resp.json;
  }
}
