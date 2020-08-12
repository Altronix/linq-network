import { LitElement, customElement, html, property, query } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';

import { styles } from '../bulma/styles';
import * as style from './ui-blocker.styles.scss';

import '../material/circular-progress/circular-progress';

@customElement('atx-ui-blocker')
class AtxUiBlocker extends LitElement {
  @property({ type: Number }) transition: number = 100;
  @property({ type: Boolean }) active: boolean = false;
  static styles = styles(style.toString());

  render() {
    const classes = { 'is-active': this.active };
    return html`
      <div class="blocker ${classMap(classes)}">
        <div class="blocked"><slot></slot></div>
        <m-circular-progress class="guard" size="large"></m-circular-progress>
      </div>
    `;
  }
}
