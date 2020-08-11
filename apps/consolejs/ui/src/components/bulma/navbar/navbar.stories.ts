import { html } from 'lit-element';
import './navbar';
import './navbar-item';
import './navbar-dropdown';
import './navbar-label';
import '../icon/icon';
import * as logo from '../../../assets/altronix_logo.png';

export default { title: 'b-navbar' };

export const basic = () => {
  return html`
    <b-navbar color="primary">
      <b-navbar-item where="left">
        <b-icon>description</b-icon>
        Documents
      </b-navbar-item>
      <b-navbar-item where="right">
        Sign In
        <b-icon>account_circle</b-icon>
      </b-navbar-item>
    </b-navbar>
  `.getHTML();
};

export const brand = () => {
  return html`
    <b-navbar color="primary">
      <b-navbar-item where="brand">
        <img src=${logo} />
      </b-navbar-item>
    </b-navbar>
  `.getHTML();
};

export const hasDropdown = () => {
  return html`
    <b-navbar color="primary">
      <b-navbar-item where="brand">
        <img src=${logo} />
      </b-navbar-item>
      <b-navbar-item where="left">
        <b-navbar-label>
          <b-icon>settings</b-icon>
          <span>Settings</span>
        </b-navbar-label>
        <b-navbar-dropdown>
          <b-navbar-item><b-icon>account_circle</b-icon>Users</b-navbar-item>
          <b-navbar-item><b-icon>device_hub</b-icon>Devices</b-navbar-item>
        </b-navbar-dropdown>
      </b-navbar-item>
      <b-navbar-item where="right" dropdown>
        <span>Sign In</span>
        <b-icon>account_circle</b-icon>
      </b-navbar-item>
    </b-navbar>
  `.getHTML();
};
