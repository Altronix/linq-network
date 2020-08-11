import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../styles';
import { Positions, Colors } from '../bulma-types';
import * as scss from './badge.styles.scss';

@customElement('b-badge')
export class BBadge extends LitElement {
  static styles = styles(scss.toString());
  @property({ type: String }) position: Positions = 'top-right';
  @property({ type: Boolean }) outline: boolean = false;
  @property({ type: String }) color: Colors = 'info';
  render() {
    const classes = {
      [`is-${this.color}`]: !!this.color,
      'is-outlined': !!this.outline,
      [`is-${this.position}`]: true
    };
    return html`
      <span class="badge ${classMap(classes)}">
        ${this.firstChild}
      </span>
    `;
  }
}
