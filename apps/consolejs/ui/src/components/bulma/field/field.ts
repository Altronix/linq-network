import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { writeAttribute, readAttribute } from '../../shared/attributes';
import { Sizes, Colors } from '../bulma-types';

import { styles } from '../styles';

@customElement('b-field')
export class BField extends LitElement {
  static styles = styles(styles.toString());
  @property({ type: String }) label?: string;
  @property({ type: String }) horizontal: boolean = false;
  @property({ type: Boolean }) centered: boolean = false;
  @property({ type: Boolean }) right: boolean = false;
  @property({ type: String }) size?: Sizes;
  @property({ type: String }) color?: Colors;
  @property({ type: Boolean }) grouped: boolean = false;
  addons: HTMLElement[] = [];
  inputs: HTMLElement[] = [];
  fields: HTMLElement[] = [];

  connectedCallback() {
    super.connectedCallback();
    this.addons = [].concat(
      Array.from(this.querySelectorAll('b-addon')),
      Array.from(this.querySelectorAll('b-addon-button')),
      Array.from(this.querySelectorAll('b-select')),
      Array.from(this.querySelectorAll('.control'))
    );
    this.inputs = [].concat(
      Array.from(this.querySelectorAll('b-input')),
      Array.from(this.querySelectorAll('b-textarea'))
    );
    this.fields = Array.from(this.querySelectorAll('b-field'));
    Array.from(this.children).forEach(i => {
      if (this.size) writeAttribute(i, 'size', this.size);
      if (this.color && !readAttribute(i, 'color')) {
        writeAttribute(i, 'color', this.color);
      }
    });
  }

  makeClasses() {
    const hasAddons = this.addons.length; /*&& !this.grouped;*/
    const grouped =
      this.addons.length || this.inputs.length >= 2 || this.fields.length;
    const isGrouped = this.grouped && grouped;
    const showLabel =
      (this.label && !(hasAddons || isGrouped)) ||
      (this.label && readAttribute(this, 'horizontal'));
    const classes = {
      field: {
        field: true,
        'has-addons': hasAddons,
        'has-addons-centered': hasAddons && this.centered,
        'has-addons-right': hasAddons && this.right,
        'is-grouped': isGrouped
      },
      ['field-label']: {
        'field-label': true,
        'is-normal': !this.size,
        [`is-${this.size}`]: !!this.size
      },
      label: {
        label: true,
        [`is-hidden`]: !showLabel,
        [`is-${this.size}`]: !!this.size
      }
    };
    return classes;
  }

  render() {
    const classes = this.makeClasses();
    return readAttribute(this, 'horizontal')
      ? html`
          <div class="field is-horizontal ${classMap(classes.field)}">
            <div class="field-label ${classMap(classes['field-label'])}">
              <label class="${classMap(classes.label)}">${this.label}</label>
            </div>
            <div class="field-body">
              ${Array.from(this.children).map(c =>
                c.classList.contains('field')
                  ? c
                  : html`
                      <div class="field">${c}</div>
                    `
              )}
            </div>
          </div>
        `
      : html`
          <div class="field ${classMap(classes.field)}">
            <label class="${classMap(classes.label)}">${this.label}</label>
            ${Array.from(this.children)}
          </div>
        `;
  }
}
