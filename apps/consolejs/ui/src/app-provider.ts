import { LitElement, html } from 'lit-element';
import { domProvider } from './components/shared/decorators';
import { container } from './ioc/container.root';

@domProvider('atx-provider', container)
export class App extends LitElement {
  render() {
    return html` <slot></slot> `;
  }
}
