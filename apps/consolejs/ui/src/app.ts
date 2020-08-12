import { customElement, property, LitElement, html, query } from 'lit-element';
import { SYMBOLS } from './ioc/constants.root';
import { domConsumer, domInject } from './components/shared/decorators';
import { RouterService } from './services/router/router.service';
import { Subscription } from 'rxjs';
import { connect } from './store/connect';
import { RootState } from './store/reducers';
import { actions } from './store/action';
import { classMap } from 'lit-html/directives/class-map';

import { styles } from './components/bulma/styles';
import * as scss from './app.styles.scss';

@domConsumer('atx-app')
export class App extends connect(LitElement) {
  static styles = styles(scss.toString());
  @property({ type: Boolean }) ready: boolean = false;
  @property({ type: String }) route: string = '';
  @domInject(SYMBOLS.ROUTER_SERVICE) router!: RouterService;
  @query('.outlet') outlet!: Element;

  connectedCallback() {
    super.connectedCallback();
  }

  stateChanged(s: RootState) {
    this.route = s.router.route;
    console.log(`State Change: ${s.router.route}`);
  }

  firstUpdated() {
    if (!this.ready) this.onReady();
  }

  go(route: string) {
    console.log(route);
    this.store.dispatch(this.actions.router.route({ route }));
  }

  onReady() {
    this.ready = true;
    let router = this.router.create(this.outlet);
    router.setRoutes([
      { path: '/', redirect: '/tcpip' },
      { path: '/tcpip', component: 'form-netw' },
      { path: '/update', component: 'form-update' }
    ]);
  }

  render() {
    const isActive = (r0: string, r1?: string) =>
      this.route === r0 || this.route === r1;
    return html`
      <atx-topnav>
        <a where="brand">Altronix</a>
      </atx-topnav>
      <div class="container mt-5">
        <div class="columns">
          <div class="column">
            <atx-device-list .height="${363}"></atx-device-list>
          </div>
        </div>
      </div>
      <div class="container">
        <atx-footer></atx-footer>
      </div>
    `;
  }
}
