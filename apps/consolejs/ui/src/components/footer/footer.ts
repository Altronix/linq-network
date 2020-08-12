import { LitElement, customElement, html, property } from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../bulma/styles';
import * as scss from './footer.styles.scss';
import '../bulma/addon/addon';

import * as logo from '../../assets/altronix_logo_large.png';
import * as facebook from '../../assets/facebook.svg';
import * as github from '../../assets/github.svg';
import * as twitter from '../../assets/twitter.svg';
import * as linkedin from '../../assets/linkedin.svg';
import * as googleLocation from '../../assets/location.svg';

@customElement('atx-footer')
export class AtxFooter extends LitElement {
  static styles = styles(scss.toString());
  render() {
    return html`
      <section class="footer align-with-container">
        <div class="columns main has-text-centered">
          <div class="column">
            <div class="is-pulled-left">
              <div class="columns">
                <div class="column">
                  <div class="container social-tabs">
                    <a class="location">
                      <img src="${googleLocation}" />
                    </a>
                    <a class="facebook">
                      <img src="${facebook}" />
                    </a>
                    <a class="github">
                      <img src="${github}" />
                    </a>
                    <a class="linkedin">
                      <img src="${linkedin}" />
                    </a>
                    <a class="twitter">
                      <img src="${twitter}" />
                    </a>
                  </div>
                </div>
              </div>
              <div class="columns">
                <div class="column">
                  <p class="is-size-7 has-text-grey">&#169; Altronix Corp</p>
                </div>
              </div>
            </div>
          </div>
          <div class="column">
            <div class="columns is-multiline company">
              <div class="column is-12">
                <p class="is-size-7 has-text-weight-bold has-text-grey">
                  Altronix.com
                </p>
              </div>
              <div class="column is-12">
                <a><p class="is-size-7 has-text-info">Contact</p></a>
              </div>
              <div class="column is-12">
                <a><p class="is-size-7 has-text-info">Help & Support</p></a>
              </div>
              <div class="column is-12">
                <a><p class="is-size-7 has-text-info">Our Team</p></a>
              </div>
            </div>
          </div>
          <div class="column">
            <div class="is-pulled-right">
              <p class="is-size-7 has-text-grey">Subscribe to Alronix News!</p>
              <b-field centered size="small">
                <b-input placeholder="Email Address">
                  <b-icon where="left">mail</b-icon>
                </b-input>
                <b-addon color="primary">Subscribe!</b-addon>
              </b-field>
            </div>
          </div>
        </div>
      </section>
    `;
  }
}
