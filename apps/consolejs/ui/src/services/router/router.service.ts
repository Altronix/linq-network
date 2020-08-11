import { injectable } from 'inversify';
import { SYMBOLS } from '../../ioc/constants.root';
import { bindTo } from '../../ioc/container.root';
import { Router } from '@vaadin/router';

interface RouteArgs {
  pathname: string;
  search?: string;
  hash?: string;
}

export class RouterService {
  constructor(private VaadinRouter: typeof Router) {}

  create(el: Element) {
    return new this.VaadinRouter(el);
  }

  route(path: string | RouteArgs): boolean {
    return this.VaadinRouter.go(path);
  }
}
