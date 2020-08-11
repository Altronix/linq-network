import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../styles';
import { Colors, Sizes, NavbarWhere, NODE_TYPES } from '../bulma-types';

@customElement('b-navbar-item')
class BNavbarItem extends LitElement {
  @property({ type: String }) where: NavbarWhere = 'left';
  @property({ type: String }) label?: string;
  @property({ type: Boolean }) arrow: boolean = false;
  @property({ type: String }) href: string = '';
  dropdown: HTMLElement[] = [];

  createRenderRoot() {
    return this;
  }

  connectedCallback() {
    super.connectedCallback();
    this.dropdown = Array.from(this.querySelectorAll('b-navbar-dropdown'));
  }

  render() {
    this.classList.add('navbar-item');
    this.classList.add(`${this.where}`);
    if (this.dropdown.length) {
      this.classList.add('has-dropdown');
      this.classList.add('is-hoverable');
    }
    const classes = {
      link: { ['navbar-link']: true, [`is-arrowless`]: !this.arrow }
    };

    // TODO - this causes some problems (dom.js start == null crash)
    const labels = Array.from(this.childNodes).map(i =>
      i.nodeType === NODE_TYPES.TEXT
        ? html`
            <span>${i}</span>
          `
        : i
    );
    return this.dropdown.length
      ? html`
          <a href="${this.href}" class="${classMap(classes.link)}">
            ${Array.from(this.querySelectorAll('b-navbar-label'))}
          </a>
          ${this.dropdown}
        `
      : html`
          ${labels}
        `;
  }
}
