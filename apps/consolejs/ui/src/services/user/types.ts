export interface User {
  firstName: string;
  lastName: string;
  phone: string;
  email: string;
  devices: any[];
  role: number;
  accessToken: string;
}

export type UserAdd = Omit<User, 'devices' | 'accessToken'> & {
  password: string;
};
