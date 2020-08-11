import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { readAttribute } from '../../shared/attributes';
import { styles } from '../styles';

import { Colors, Sizes, NODE_TYPES } from '../bulma-types';

@customElement('b-addon')
export class BAddon extends LitElement {
  @property({ type: Boolean }) fullwidth: boolean = false;
  @property({ type: Boolean }) static: boolean = false;
  @property({ type: String }) color?: Colors;
  @property({ type: String }) size?: Sizes;

  createRenderRoot() {
    return this;
  }

  constructor() {
    super();
    this.classList.add('control');
    if (this.fullwidth) this.classList.add('is-expanded');
  }
  render() {
    const classes = {
      [`is-${this.color}`]: !!this.color,
      [`is-static`]: !!this.static,
      [`is-${this.size}`]: !!this.size,
      button: true
    };
    return html`
      <a class="${classMap(classes)}">${this.firstChild}</a>
    `;
  }
}

@customElement('b-addon-button')
export class BAddonButton extends LitElement {
  @property({ type: Boolean }) expanded: boolean = false;
  @property({ type: String }) color?: Colors;
  @property({ type: String }) size?: Sizes;
  @property({ type: Boolean }) disabled?: boolean = false;
  elementChildren: Element[] = [];
  constructor() {
    super();
    this.classList.add('control');
  }

  createRenderRoot() {
    return this;
  }

  render() {
    let elementChildren = Array.from(this.childNodes).map(e =>
      e.nodeType === NODE_TYPES.TEXT
        ? html`
            <span>${e}</span>
          `
        : e
    );
    const classes = {
      [`is-${this.color}`]: !!this.color,
      [`is-${this.size}`]: !!this.size,
      [`is-expanded`]: !!this.expanded
    };
    return html`
      <a ?disabled="${this.disabled}" class="button ${classMap(classes)}">
        ${elementChildren}
      </a>
    `;
  }
}
