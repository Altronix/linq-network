import { customElement, property, LitElement, html, query } from 'lit-element';
import { SYMBOLS } from './ioc/constants.root';
import { domConsumer, domInject } from './components/shared/decorators';
import { RouterService } from './services/router/router.service';
import { Subscription } from 'rxjs';
import { connect } from './store/connect';
import { RootState } from './store/reducers';
import { classMap } from 'lit-html/directives/class-map';
import { DeviceListTableClickEvent } from './components/device-list/device-list';

import { styles } from './components/bulma/styles';
import * as scss from './app.styles.scss';

@domConsumer('atx-app')
export class App extends connect(LitElement) {
  static styles = styles(scss.toString());
  @property({ type: Boolean }) ready: boolean = false;
  @property({ type: String }) route: string = '';
  @property({ type: Boolean }) show: string = '';
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

  deviceListTableClick(e: CustomEvent<DeviceListTableClickEvent>) {
    this.show = e.detail.device.serial;
  }

  close() {
    this.show = '';
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
            <atx-device-list
              @device-list-table-click="${this.deviceListTableClick}"
              .height="${363}"
            ></atx-device-list>
          </div>
        </div>
      </div>
      <div class="container">
        <div class="outlet"><slot></slot></div>
        <atx-footer></atx-footer>
      </div>
      <b-modal @b-close="${this.close}" ?show="${this.show.length > 0}"">
        <div class="is-clipped">
          <div class="box">
            <p class="popup-title mb-5">
              Submit network settings for ${this.show}
            </p>
            <atx-form-netw></atx-form-netw>
          </div>
        </div>
      </b-modal>
    `;
  }
}
