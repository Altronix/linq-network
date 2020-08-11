import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { readAttribute } from '../../shared/attributes';
import { styles } from '../styles';

import { IconWhere, Sizes, Colors } from '../bulma-types';

@customElement('b-icon')
export class BIcon extends LitElement {
  static styles = styles(styles.toString());
  @property({ type: String }) where: IconWhere = 'left';
  @property({ type: String }) kind: string = 'material-icons';
  @property({ type: String }) icon: string | undefined = undefined;
  @property({ type: String }) size: Sizes | undefined = undefined;
  @property({ type: String }) color: Colors | undefined = undefined;

  constructor() {
    super();
    this.classList.add('b-icon');
    this.classList.add('icon');
    this.classList.add(`is-${this.where}`);
  }

  createRenderRoot() {
    return this;
  }

  render() {
    this.classList.add(`is-${this.where}`);
    if (this.size) this.classList.add(`is-${this.size}`);
    if (this.color) this.classList.add(`has-text-${this.color}`);
    return html`
      <i class="${this.kind}">
        ${this.icon ? this.icon : this.firstChild}
      </i>
    `;
  }
}
