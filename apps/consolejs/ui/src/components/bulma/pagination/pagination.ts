import { LitElement, customElement, html, property } from 'lit-element';
import { readAttribute, writeAttribute } from '../../shared/attributes';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../styles';

import { Colors, Sizes, NavbarWhere } from '../bulma-types';

import * as scss from './pagination.styles.scss';

type Events = 'prev' | 'next';

@customElement('b-pagination')
export class BPagination extends LitElement {
  static styles = styles(scss.toString());
  @property({ type: Number }) total: number = 0;
  @property({ type: Boolean }) simple: boolean = false;
  @property({ type: String }) size?: Sizes;
  @property({ type: Number, attribute: 'per-page' }) perPage: number = 0;
  @property({ type: Number }) current: number = 1;
  @property({ type: Number, attribute: 'buttons-after' })
  buttonsAfter: number = 3;
  @property({ type: Number, attribute: 'buttons-before' })
  buttonsBefore: number = 3;
  range: number = 0;

  emit(event: Events) {
    let ev = new CustomEvent(`b-${event}`, {
      bubbles: true,
      composed: true,
      detail: { current: this.current + 1 }
    });
    this.dispatchEvent(ev);
  }

  next(e: Event) {
    e.stopPropagation();
    if (!(this.range === this.total)) {
      this.dispatchEvent(
        new CustomEvent('b-next', {
          bubbles: true,
          composed: true,
          detail: { current: this.current + 1 }
        })
      );
    }
  }

  prev(e: Event) {
    e.stopPropagation();
    if (!(this.current === 1)) {
      this.dispatchEvent(
        new CustomEvent('b-prev', {
          bubbles: true,
          composed: true,
          detail: { current: this.current + 1 }
        })
      );
    }
  }

  render() {
    const classes = { [`is-${this.size}`]: !!this.size };
    const pageCount = Math.ceil(this.total / this.perPage);
    this.range = Math.min(this.current + this.perPage, this.total);
    let firstItem = this.current * (this.perPage - this.perPage + 1);
    if (firstItem < 0) firstItem = 0;
    return html`
      <div class="pagination ${classMap(classes)}">
        <a
          class="pagination-previous"
          ?disabled="${this.current === 1}"
          @click="${this.prev}"
        >
          Previous
        </a>
        <a
          class="pagination-previous"
          ?disabled="${this.range === this.total}"
          @click="${this.next}"
        >
          Next page
        </a>
        ${this.simple
          ? this.perPage === 1
            ? html`
                <small class="is-simple">${firstItem} / ${this.total}</small>
              `
            : html`
                <small class="is-simple">
                  ${firstItem} - ${this.range} / ${this.total}
                </small>
              `
          : html`
              <ul class="pagination-list">
                <li><a class="pagination-link">1</a></li>
                <li><a class="pagination-link">2</a></li>
                <li><a class="pagination-link">3</a></li>
                <li><a class="pagination-ellipse">&hellip;</a></li>
                <li><a class="pagination-link">${this.current}</a></li>
                <li><a class="pagination-link">11</a></li>
                <li><a class="pagination-link">12</a></li>
                <li><a class="pagination-ellipse">&hellip;</a></li>
                <li><a class="pagination-link">100</a></li>
                <li><a class="pagination-link">101</a></li>
                <li><a class="pagination-link">102</a></li>
              </ul>
            `}
      </div>
    `;
  }
}
