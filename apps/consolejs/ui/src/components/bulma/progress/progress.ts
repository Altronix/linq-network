import { LitElement, customElement, html, property, query } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { readAttribute } from '../../shared/attributes';
import { ifDefined } from 'lit-html/directives/if-defined.js';
import { Sizes, Colors, TextFieldType } from '../bulma-types';
import { styles } from '../styles';
import * as scss from './progress.styles.scss';

@customElement('b-progress')
class BProgress extends LitElement {
  static styles = styles(scss.toString());
  @property({ type: String }) color?: Colors;
  @property({ type: String }) size?: Sizes;
  @property({ type: Boolean }) indeterminate: boolean = false;
  @property({ type: Number }) max: number = 100;
  @property({ type: Number }) value: number = 0;
  @property({ type: Boolean }) fullwidth: boolean = false;
  render() {
    const classes = {
      [`is-${this.color}`]: !!this.color,
      [`is-${this.size}`]: !!this.size,
      [`is-fullwidth`]: !!this.fullwidth
    };
    return html`
      <progress
        class="progress ${classMap(classes)}"
        value="${this.value}"
        max="${this.max}"
      ></progress>
    `;
  }
}
