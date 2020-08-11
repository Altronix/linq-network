import { Container } from 'inversify';
import { makeDecorators } from '../components/shared/decorators';
import serviceContainer from './service-container';

export const container = new Container();
container.load(serviceContainer);
export const { bind, bindTo, lazyInject } = makeDecorators(container);
