import { bulma } from './bulma';
import { unsafeCSS } from 'lit-element';

export const styles = (style: string) => [unsafeCSS(style), bulma()];
export default styles;

// Caller must implicitly import bulma (this allows component to override)
export const stylesCustom = (style: string) => [unsafeCSS(style)];
