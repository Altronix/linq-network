import { LitElement, customElement, html } from 'lit-element';
import { styles } from './styles';

// This component isn't used anywhere however it is kept as example way to render
// child elements with out a slot when you are in light dom.
@customElement('bulma-base')
export class BulmaBase extends LitElement {
  static styles = styles(styles.toString());
  elementChildren: Array<Element> = [];

  createRenderRoot() {
    return this;
  }

  connectedCallback() {
    this.elementChildren = Array.from(this.children);
  }

  render() {
    return html`
      ${this.elementChildren}
    `;
  }
}
