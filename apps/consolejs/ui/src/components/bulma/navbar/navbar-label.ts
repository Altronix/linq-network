import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { Colors, Sizes, NavbarWhere, NODE_TYPES } from '../bulma-types';

@customElement('b-navbar-label')
class BNavbarLabel extends LitElement {
  createRenderRoot() {
    return this;
  }

  render() {
    this.classList.add('b-navbar-label');
    const children = Array.from(this.childNodes).map(i =>
      i.nodeType === NODE_TYPES.TEXT
        ? html`
            <span>${i}</span>
          `
        : i
    );
    return html`
      ${children}
    `;
  }
}
