import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../bulma/styles';
import { Device } from '../../store/devices/state';
import { connect } from '../../store/connect';
import { RootState } from '../../store/reducers';
import * as scss from './device-list.styles.scss';

export interface DeviceListTableClickEvent {
  device: Device;
}

@customElement('atx-device-list')
export class AtxDeviceList extends connect(LitElement) {
  static styles = styles(scss.toString());
  @property({ type: Number }) height: number = 0;
  @property({ type: Number }) selected: number = -1;
  @property({ type: Boolean }) loading: boolean = false;
  @property({ type: Boolean }) polling: boolean = false;
  @property({ type: Number }) start: number = 0;
  @property({ type: Number }) count: number = 0;
  @property({ type: Number }) perPage: number = 10;
  @property({ type: String }) popup: string = '';
  @property({ type: Array }) devices: Device[] = [];

  calcLastSeen(d: Device) {
    const secs = Math.floor(new Date().getTime() / 1000) - d.lastSeen;
    return html` <p>${secs}s</p> `;
  }

  stateChanged(state: RootState) {
    this.devices = state.devices.devices;
  }

  tableClick(device: Device) {
    this.dispatchEvent(
      new CustomEvent<DeviceListTableClickEvent>('device-list-table-click', {
        bubbles: true,
        composed: true,
        detail: { device }
      })
    );
  }

  renderTable() {
    const c = {
      table: {
        table: true,
        ['is-bordered']: false,
        ['is-striped']: true,
        ['is-narrow']: true,
        ['is-hoverable']: true,
        ['is-fullwidth']: true
      },
      column: (numeric?: boolean) => {
        return {
          'is-numeric': !!numeric
        };
      },
      row: (idx: number) => {
        return {
          'is-selected': idx === this.selected
        };
      }
    };
    return html`
      <div
        class="table-container"
        style="${this.height ? `height:${this.height}px` : ``}"
      >
        <table class="${classMap(c.table)}">
          <thead>
            <tr>
              <th class="${classMap(c.column(true))}">Idx</th>
              <th class="${classMap(c.column())}">Serial</th>
              <th class="${classMap(c.column())}">Site</th>
              <th class="${classMap(c.column())}">Product</th>
              <th class="${classMap(c.column())}">Version</th>
              <th class="${classMap(c.column())}">MAC</th>
              <th class="${classMap(c.column())}">Last Seen</th>
              <th class="${classMap(c.column(true))}">View</th>
            </tr>
          </thead>
          <tbody>
            ${[...this.devices].splice(0, 10).map((d, n) => {
              const idx = this.start + 1 + n;
              return html`
                <tr
                  class="${classMap(c.row(idx))}"
                  @click="${() => this.tableClick(d)}"
                >
                  <td class="${classMap(c.column(true))}">${idx}</td>
                  <td class="${classMap(c.column())}">
                    <div class="truncate">
                      ${d.serial}
                    </div>
                  </td>
                  <td class="${classMap(c.column())}">${d.siteId}</td>
                  <td class="${classMap(c.column())}">${d.product}</td>
                  <td class="${classMap(c.column())}">${d.prjVersion}</td>
                  <td class="${classMap(c.column())}">${d.mac}</td>
                  <td class="${classMap(c.column())}">
                    <div>
                      ${this.calcLastSeen(d)}
                    </div>
                  </td>
                  <td class="${classMap(c.column(true))}">
                    <span class="is-hidden">align</span>
                    <b-icon size="small" color="info">router</b-icon>
                  </td>
                </tr>
              `;
            })}
          </tbody>
        </table>
      </div>
    `;
  }

  render() {
    return html`
      <div class="columns is-desktop">
        <div class="column">
          <b-field size="small">
            <b-select>
              <b-icon>search</b-icon>
              <option>Serial</option>
              <option>Product</option>
              <option>Mac</option>
            </b-select>
            <b-input expanded placeholder="search"></b-input>
            <b-addon-button color="warning">
              <b-icon>search</b-icon>
            </b-addon-button>
          </b-field>
        </div>
      </div>
      <div class="columns">
        <div class="column">
          <atx-ui-blocker ?active="${this.loading}">
            ${this.renderTable()}
          </atx-ui-blocker>
        </div>
      </div>
      <div class="columns">
        <div class="column">
          <b-pagination
            simple
            size="small"
            current="${this.start + 1}"
            total="${this.count}"
            per-page="${this.perPage}"
          >
          </b-pagination>
        </div>
      </div>
    `;
  }
}
