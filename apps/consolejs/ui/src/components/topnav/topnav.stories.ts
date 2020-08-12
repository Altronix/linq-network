import { html } from 'lit-element';

export default { title: 'atx-topnav' };

export const basic = () => html` <atx-topnav></atx-topnav> `.getHTML();

export const wide = () =>
  html`
    <atx-topnav wide>
      <a where="brand" class="is-size-5">Altronix</a>
      <a where="right">Logout</a>
    </atx-topnav>
  `.getHTML();
