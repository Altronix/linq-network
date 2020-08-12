import 'reflect-metadata';
import './app';
import './app-provider';
import './store/connect';
import './components/bulma/addon/addon';
import './components/bulma/badge/badge';
import './components/bulma/bulma-base';
import './components/bulma/bulma';
import './components/bulma/field/field';
import './components/bulma/icon/icon';
import './components/bulma/input/input';
import './components/bulma/modal/modal';
import './components/bulma/navbar/navbar';
import './components/bulma/navbar/navbar-label';
import './components/bulma/navbar/navbar-item';
import './components/bulma/navbar/navbar-dropdown';
import './components/bulma/pagination/pagination';
import './components/bulma/progress/progress';
import './components/bulma/select/select';
import './components/bulma/table/table';
import './components/bulma/textarea/textarea';
import './components/material/circular-progress/circular-progress';
import './components/footer/footer';
import './components/forms/netw/form-netw';
import './components/forms/update/form-update';
import './components/device-list/device-list';
import './components/topnav/topnav';
import './components/ui-blocker/ui-blocker';

import { customElement, LitElement, html } from 'lit-element';
@customElement('atx-root')
export class AppRoot extends LitElement {
  render() {
    return html`
      <atx-provider>
        <atx-app></atx-app>
      </atx-provider>
    `;
  }
}
