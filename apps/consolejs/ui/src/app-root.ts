import 'reflect-metadata';
import './app';
import './app-provider';
import './store/connect';
import { customElement, LitElement, html } from 'lit-element';
@customElement('atx-root')
export class AppRoot extends LitElement {
  render() {
    return html`
      <atx-provider>
        <atx-app></atx-app>
      </atx-provider>
    `;
  }
}
