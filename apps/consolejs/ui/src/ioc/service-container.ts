import { SYMBOLS } from "./constants.root";
// import { UserService } from '../services/user/user.service';
import { IoService } from "../services/io/io.service";
import { RouterService } from "../services/router/router.service";
import { ContainerModule } from "inversify";
import { rootReducer, RootState } from "../store/reducers";
import { rootEpics } from "../store/epics";
import { RootActions, actions } from "../store/action";
import { Dependencies } from "../store/dependencies";
import { createStore, applyMiddleware, combineReducers } from "redux";
import { createEpicMiddleware, combineEpics } from "redux-observable";
import { Router } from "@vaadin/router";

// createEpicMiddleware Type params needs to be explicit when passing
// dependencies...
// See: https://github.com/redux-observable/redux-observable/issues/592
export default new ContainerModule((bind) => {
  const io = new IoService(fetch.bind(window));
  // const users = new UserService(io);
  const router = new RouterService(Router);
  const epicMiddleware = createEpicMiddleware<
    RootActions,
    RootActions,
    RootState,
    Dependencies
  >({
    dependencies: { /* users, */ router, io },
  });
  const store = createStore(rootReducer, applyMiddleware(epicMiddleware));
  epicMiddleware.run(rootEpics);
  bind(SYMBOLS.ROUTER_SERVICE).toDynamicValue(() => router);
  bind(SYMBOLS.STORE_SERVICE).toDynamicValue(() => store);
  bind(SYMBOLS.ACTION_SERVICE).toDynamicValue(() => actions);
});
