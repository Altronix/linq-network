// import { UserService } from '../services/user/user.service';
import { RouterService } from '../services/router/router.service';
import { IoService } from '../services/io/io.service';

// These are injected into all Epics...
export type Dependencies = {
  // users: UserService;
  router: RouterService;
  io: IoService;
};
