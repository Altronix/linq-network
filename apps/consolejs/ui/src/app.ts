import { customElement, property, LitElement, html, query } from 'lit-element';
import { SYMBOLS } from './ioc/constants.root';
import { domConsumer, domInject } from './components/shared/decorators';
import { RouterService } from './services/router/router.service';
import { Subscription } from 'rxjs';
import { connect } from './store/connect';
import { RootState } from './store/reducers';
import { actions } from './store/action';

import { styles } from './components/bulma/styles';
import * as scss from './app.styles.scss';

@domConsumer('atx-app')
export class App extends connect(LitElement) {
  static styles = styles(scss.toString());
  @property({ type: Boolean }) ready: boolean = false;
  @domInject(SYMBOLS.ROUTER_SERVICE) router!: RouterService;
  @query('.outlet') outlet!: Element;

  connectedCallback() {
    super.connectedCallback();
  }

  stateChanged(s: RootState) {}

  onReady() {
    let router = this.router.create(this.outlet);
    router.setRoutes([
      { path: '/', redirect: '/dashboard' },
      { path: '/home', component: 'atx-home' },
      { path: '/logout', component: 'atx-logout' },
      { path: '/docs', component: 'atx-docs' },
      {
        path: '/dashboard',
        component: 'atx-dashboard',
        children: [
          { path: '/', redirect: '/dashboard/update' },
          { path: '/main', component: 'atx-dashboard-main' },
          { path: '/about', component: 'atx-dashboard-about' },
          { path: '/account', component: 'atx-dashboard-account' },
          { path: '/mail', component: 'atx-dashboard-mail' },
          { path: '/lock', component: 'atx-dashboard-lock' },
          { path: '/devices', component: 'atx-dashboard-devices' },
          { path: '/network', component: 'atx-dashboard-network' },
          { path: '/power', component: 'atx-dashboard-power' },
          { path: '/update', component: 'atx-dashboard-update-container' }
        ]
      }
    ]);
  }

  render() {
    return html` <div class="outlet"></div> `;
  }
}
