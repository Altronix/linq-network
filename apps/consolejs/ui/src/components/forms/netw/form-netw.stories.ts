import { html } from 'lit-element';

export default { title: 'atx-form-netw' };

export const basic = () => html` <atx-form-netw></atx-form-netw> `.getHTML();
export const loading = () =>
  html` <atx-form-netw loading></atx-form-netw> `.getHTML();
