import * as bulmaScss from './bulma-scss.scss';
import { unsafeCSS } from 'lit-element';

export const bulma = () => unsafeCSS(bulmaScss.toString());
export default bulma;
