import { html } from 'lit-element';
import { BBadge } from './badge';

export default { title: 'b-badge' };

export const basic = () => {
  return html`
    <button class="button">
      <b-badge>8</b-badge>
      Foo
    </button>
  `.getHTML();
  // return html`
  //   <b-badge></b-badge>
  // `.getHTML();
};
