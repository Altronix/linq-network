import { LitElement, customElement, html, property, query } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../../bulma/styles';
import { connect } from '../../../store/connect';
import { RootState } from '../../../store/reducers';
import { M5Network, initial } from '../../../store/m5-netw/state';
import * as scss from './form-m5netw.styles.scss';

@customElement('atx-form-m5netw')
export class FormNetw extends connect(LitElement) {
  @property({ type: Boolean }) loading: boolean = false;
  @property({ type: Boolean }) success: boolean = false;
  @property({ type: Boolean }) error: boolean = false;
  @property({ type: String }) serial: string = '';
  @property({ type: String }) message: string = '';
  data: M5Network = initial.data;

  stateChanged(s: RootState) {
    this.loading = s.m5Network.loading;
    this.data = s.m5Network.data;
    if (s.m5Network.result === 'SUCCESS') {
      this.success = true;
    } else if (s.m5Network.result === 'ERROR') {
      this.error = true;
    } else {
      this.success = this.error = false;
    }
  }

  submit() {
    this.store.dispatch(this.actions.m5network.fetch({ serial: this.serial }));
  }

  render() {
    return html`
      <atx-ui-blocker ?active="${this.loading}">
        <b-field size="small" horizontal label="Method">
          <b-field>
            <b-select
              expanded
              @input="${(e: any) => (this.data.meth = e.target.value)}"
            >
              <option>Manual</option>
              <option>DHCP</option>
            </b-select>
          </b-field>
        </b-field>
        <b-field size="small" horizontal label="IPv4 Address">
          <b-field>
            <b-input
              expanded
              placeholder="192.168.0.2"
              @input="${(e: any) => (this.data.ip = e.target.value)}"
            ></b-input>
          </b-field>
        </b-field>
        <b-field size="small" horizontal label="Subnet Mask">
          <b-field>
            <b-input
              placeholder="255.255.255.0"
              @input="${(e: any) => (this.data.sn = e.target.value)}"
            ></b-input>
          </b-field>
        </b-field>
        <b-field size="small" horizontal label="Gateway">
          <b-field>
            <b-input
              placeholder="192.168.0.1"
              @input="${(e: any) => (this.data.gw = e.target.value)}"
            ></b-input>
          </b-field>
        </b-field>
        <b-field size="small" horizontal label="DNS">
          <b-field>
            <b-input
              placeholder="4.4.8.8"
              @input="${(e: any) => (this.data.dns0 = e.target.value)}"
            ></b-input>
          </b-field>
        </b-field>
        <b-field size="small" horizontal label="Hostname">
          <b-field>
            <b-input
              placeholder="LinQM5"
              @input="${(e: any) => (this.data.hn = e.target.value)}"
            ></b-input>
          </b-field>
        </b-field>
        <b-field size="small" horizontal>
          <b-addon-button color="primary" @click="${this.submit}">
            Submit
          </b-addon-button>
        </b-field>
      </atx-ui-blocker>
    `;
  }
}
