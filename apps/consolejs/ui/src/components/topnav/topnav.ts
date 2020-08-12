import { LitElement, customElement, html, property, query } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../bulma/styles';
import { domConsumer, domInject } from '../../components/shared/decorators';
import { Subscription } from 'rxjs';
import { connect } from '../../store/connect';
import { RootState } from '../../store/reducers';
import * as scss from './topnav.styles.scss';
import * as logo from '../../assets/altronix.png';

@customElement('atx-topnav')
export class AtxTopnav extends connect(LitElement) {
  static styles = styles(scss.toString());
  @property({ type: Boolean }) wide: boolean = false;
  @property({ type: String }) user: string = '';

  stateChanged(state: RootState) {}

  render() {
    return html`
      <b-navbar color="primary" ?wide="${this.wide}">
        <b-navbar-item where="brand">
          <a href="/home"><img src="${logo}" height="32px" /></a>
        </b-navbar-item>
        ${Array.from(this.children)}
        <!-- -->
        <!-- -->
        <!-- -->
      </b-navbar>
    `;
  }
}
