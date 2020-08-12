import { LitElement, customElement, html, property, query } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../../bulma/styles';
import * as scss from './form-netw.styles.scss';

@customElement('form-netw')
export class FormNetw extends LitElement {
  render() {
    return html`
      <b-field size="small" horizontal label="Method">
        <b-field>
          <b-select expanded>
            <option>Manual</option>
            <option>DHCP</option>
          </b-select>
        </b-field>
      </b-field>
      <b-field size="small" horizontal label="IPv4 Address">
        <b-field>
          <b-input expanded placeholder="192.168.0.2"></b-input>
        </b-field>
      </b-field>
      <b-field size="small" horizontal label="Subnet Mask">
        <b-field>
          <b-input placeholder="255.255.255.0"></b-input>
        </b-field>
      </b-field>
      <b-field size="small" horizontal label="Gateway">
        <b-field>
          <b-input placeholder="192.168.0.1"></b-input>
        </b-field>
      </b-field>
      <b-field size="small" horizontal label="DNS">
        <b-field>
          <b-input placeholder="4.4.8.8"></b-input>
        </b-field>
      </b-field>
      <b-field size="small" horizontal label="Hostname">
        <b-field>
          <b-input placeholder="LinQM5"></b-input>
        </b-field>
      </b-field>
      <b-field size="small" horizontal>
        <b-addon-button color="primary">Submit</b-addon-button>
      </b-field>
    `;
  }
}
