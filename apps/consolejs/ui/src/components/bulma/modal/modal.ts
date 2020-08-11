import { LitElement, customElement, html, property } from 'lit-element';
import { readAttribute, writeAttribute } from '../../shared/attributes';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../styles';

import * as scss from './modal.styles.scss';

@customElement('b-modal')
export class BModal extends LitElement {
  static styles = styles(scss.toString());

  @property({ type: Boolean }) show: boolean = false;

  close() {
    this.dispatchEvent(new Event('b-close', { bubbles: true, composed: true }));
  }

  render() {
    const classes = { ['is-active']: this.show };
    return html`
      <div class="modal ${classMap(classes)}">
        <div @click="${() => this.close()}" class="modal-background"></div>
        <div class="modal-content">
          <slot></slot>
        </div>
        <button @click="${() => this.close()}" class="modal-close is-large">
          close
        </button>
      </div>
    `;
  }
}
