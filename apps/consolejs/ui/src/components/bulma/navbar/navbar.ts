import { LitElement, customElement, html, property } from 'lit-element';
import { readAttribute, writeAttribute } from '../../shared/attributes';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../styles';

import { Colors, Sizes, NavbarWhere } from '../bulma-types';

import * as scss from './navbar.styles.scss';

@customElement('b-navbar')
class BNavbar extends LitElement {
  static styles = styles(scss.toString());
  @property({ type: String }) color?: Colors;
  @property({ type: Boolean }) wide?: boolean = false;
  brand: Element[] = [];
  start: Element[] = [];
  end: Element[] = [];

  connectedCallback() {
    super.connectedCallback();
    Array.from(this.children).forEach(i => {
      const where = readAttribute<NavbarWhere>(i, 'where');
      if (where === 'brand') {
        this.brand.push(i);
      } else if (where === 'right') {
        this.end.push(i);
      } else {
        this.start.push(i);
      }
      if (!i.classList.contains('navbar-item')) i.classList.add('navbar-item');
    });
  }

  render() {
    const classes = {
      navbar: { navbar: true, [`is-${this.color}`]: !!this.color }
    };
    return html`
      <div class="${classMap(classes.navbar)}">
        <div class="${this.wide ? 'navbar-container' : 'container'}">
          <div class="navbar-brand">
            ${this.brand}
            <a
              class="navbar-burger burger"
              role="button"
              aria-label="menu"
              aria-expanded="false"
              data-target="brandButton"
            >
              <span aria-hidden="true"></span>
              <span aria-hidden="true"></span>
              <span aria-hidden="true"></span>
            </a>
          </div>
          <div class="navbar-menu">
            <div class="navbar-start">${this.start}</div>
            <div class="navbar-end">${this.end}</div>
          </div>
        </div>
      </div>
    `;
  }
}
