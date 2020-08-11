import { html } from 'lit-element';
import '../field/field';
import '../icon/icon';
import '../input/input';
import '../addon/addon';
import './select';

export default { title: 'b-select' };

export const basic = () => {
  return html`
    <b-field label="Basic">
      <b-select>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
    </b-field>
  `.getHTML();
};

export const withIcons = () => {
  return html`
    <b-field label="With Icons">
      <b-select>
        <b-icon where="left">mail</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
    </b-field>
  `.getHTML();
};

export const withHelp = () => {
  return html`
    <b-field label="With Help" color="danger">
      <b-select help="Select where you are from">
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
    </b-field>
  `.getHTML();
};

export const withAddon = () => {
  return html`
    <b-field label="With Addon">
      <b-addon color="info">FOO</b-addon>
      <b-select>
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
      <b-input placeholder="test">
        <b-icon>mail</b-icon>
      </b-input>
      <b-addon static color="info">@gmail.com</b-addon>
      <b-addon-button color="success">
        <b-icon>search</b-icon>
        Search
      </b-addon-button>
    </b-field>
  `.getHTML();
};

export const withAddonOther = () => {
  return html`
    <b-field>
      <b-select>
        <b-icon where="left">search</b-icon>
        <option>Name</option>
        <option>Email</option>
        <option>Role</option>
      </b-select>
      <b-input placeholder="search">
        <b-icon>mail</b-icon>
      </b-input>
      <b-addon-button>
        <b-icon>search</b-icon>
        Search
      </b-addon-button>
    </b-field>
    <b-field>
      <b-select>
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
      <b-input placeholder="test">
        <b-icon>mail</b-icon>
      </b-input>
      <b-addon static color="info">@gmail.com</b-addon>
      <b-addon-button color="success">
        <b-icon>search</b-icon>
        Search
      </b-addon-button>
    </b-field>
  `.getHTML();
};
export const withAddonFieldMods = () => {
  return html`
    <b-field label="Field Mods" color="success" size="small">
      <b-addon>FOO</b-addon>
      <b-select>
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
      <b-addon static color="info">@gmail.com</b-addon>
    </b-field>
  `.getHTML();
};

export const withAddonFieldModsMultiColor = () => {
  return html`
    <b-field label="Field Mods Multi Color" color="success" size="small">
      <b-addon color="info">FOO</b-addon>
      <b-select>
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
      <b-addon static color="info">@gmail.com</b-addon>
    </b-field>
  `.getHTML();
};

export const withAddonFieldModsExpanded = () => {
  return html`
    <b-field label="Field Mods Expanded" color="success" size="small">
      <b-addon color="info">FOO</b-addon>
      <b-select expanded>
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
      <b-addon static color="info">@gmail.com</b-addon>
    </b-field>
  `.getHTML();
};

export const withGrouped = () => {
  return html`
    <b-field label="Grouped" grouped>
      <b-addon color="info">FOO</b-addon>
      <b-select>
        <b-icon where="left">account_circle</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
      <b-input placeholder="addons">
        <b-icon where="left">mail</b-icon>
      </b-input>
      <b-addon static color="info">@gmail.com</b-addon>
      <b-input placeholder="email">
        <b-icon where="left">account_circle</b-icon>
      </b-input>
    </b-field>
  `.getHTML();
};

export const horizontalBasic = () => {
  return html`
    <b-field horizontal label="Horizontal Basic">
      <b-select>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
    </b-field>
  `.getHTML();
};

export const horizontalWithIcons = () => {
  return html`
    <b-field horizontal label="With Icons">
      <b-select>
        <b-icon where="left">mail</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
    </b-field>
  `.getHTML();
};

export const horizontalWithHelp = () => {
  return html`
    <b-field horizontal label="With Help">
      <b-select color="danger" help="Where are you from?">
        <b-icon where="left">mail</b-icon>
        <option>Texas</option>
        <option>Main</option>
        <option>New York</option>
        <option>California</option>
      </b-select>
    </b-field>
  `.getHTML();
};

export const horizontalWithAddon = () => {
  return html`
    <b-field horizontal label="With Addon">
      <b-field>
        <b-addon color="info">FOO</b-addon>
        <b-select help="Where are you from?">
          <b-icon where="left">mail</b-icon>
          <option>Texas</option>
          <option>Main</option>
          <option>New York</option>
          <option>California</option>
        </b-select>
        <b-addon static color="info">@gmail.com</b-addon>
      </b-field>
    </b-field>
  `.getHTML();
};

export const horizontalWithAddonFieldMods = () => {
  return html`
    <b-field horizontal label="Field Mods" color="success" size="small">
      <b-field>
        <b-addon>FOO</b-addon>
        <b-select help="Where are you from?">
          <b-icon where="left">mail</b-icon>
          <option>Texas</option>
          <option>Main</option>
          <option>New York</option>
          <option>California</option>
        </b-select>
        <b-addon static color="info">@gmail.com</b-addon>
      </b-field>
    </b-field>
  `.getHTML();
};

export const horizontalWithAddonFieldModsMultiColor = () => {
  return html`
    <b-field
      horizontal
      label="Field Mods MultiColor"
      color="success"
      size="small"
    >
      <b-field>
        <b-addon color="info">FOO</b-addon>
        <b-select>
          <b-icon where="left">mail</b-icon>
          <option>Texas</option>
          <option>Main</option>
          <option>New York</option>
          <option>California</option>
        </b-select>
        <b-addon static color="info">@gmail.com</b-addon>
      </b-field>
    </b-field>
  `.getHTML();
};

export const horizontalWithAddonFieldModsExpanded = () => {
  return html`
    <b-field
      horizontal
      label="Field Mods Expanded"
      color="success"
      size="small"
    >
      <b-field>
        <b-addon color="info">FOO</b-addon>
        <b-select expanded>
          <b-icon where="left">mail</b-icon>
          <option>Texas</option>
          <option>Main</option>
          <option>New York</option>
          <option>California</option>
        </b-select>
        <b-addon static color="info">@gmail.com</b-addon>
      </b-field>
    </b-field>
  `.getHTML();
};

export const horizontalWithGrouped = () => {
  return html`
    <b-field horizontal label="Horizontal Grouped">
      <b-field grouped>
        <b-addon color="info">FOO</b-addon>
        <b-input placeholder="addons">
          <b-icon where="left">mail</b-icon>
        </b-input>
        <b-select>
          <b-icon where="left">mail</b-icon>
          <option>Texas</option>
          <option>Main</option>
          <option>New York</option>
          <option>California</option>
        </b-select>
        <b-addon static color="info">@gmail.com</b-addon>
        <b-input placeholder="email">
          <b-icon where="left">account_circle</b-icon>
        </b-input>
      </b-field>
    </b-field>
  `.getHTML();
};
