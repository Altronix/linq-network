import {
  LitElement,
  unsafeCSS,
  customElement,
  html,
  property,
  query
} from 'lit-element';
import { classMap } from 'lit-html/directives/class-map';

import * as style from './circular-progress.styles.scss';

@customElement('m-circular-progress')
class CircularProgress extends LitElement {
  static styles = [unsafeCSS(style)];
  @property({ type: String }) theme = 'default';
  @property({ type: String }) size: 'small' | 'medium' | 'large' = 'small';

  renderSvg(
    size: 'small' | 'medium' | 'large',
    kind: 'determinate' | 'indeterminate'
  ) {
    const svg = {
      small: {
        determinate: html`
          <svg
            class="mdc-circular-progress__determinate-circle-graphic"
            viewBox="0 0 24 24"
            xmlns="http://www.w3.org/2000/svg"
          >
            <circle
              class="mdc-circular-progress__determinate-circle"
              cx="12"
              cy="12"
              r="8.75"
              stroke-dasharray="54.978"
              stroke-dashoffset="54.978"
            />
          </svg>
        `,
        indeterminate: html`
          <svg
            class="mdc-circular-progress__indeterminate-circle-graphic"
            viewBox="0 0 24 24"
            xmlns="http://www.w3.org/2000/svg"
          >
            <circle
              cx="12"
              cy="12"
              r="8.75"
              stroke-dasharray="54.978"
              stroke-dashoffset="27.489"
            />
          </svg>
        `
      },
      medium: {
        determinate: html`
          <svg
            class="mdc-circular-progress__determinate-circle-graphic"
            viewBox="0 0 32 32"
            xmlns="http://www.w3.org/2000/svg"
          >
            <circle
              class="mdc-circular-progress__determinate-circle"
              cx="16"
              cy="16"
              r="12.5"
              stroke-dasharray="78.54"
              stroke-dashoffset="78.54"
            />
          </svg>
        `,
        indeterminate: html`
          <svg
            class="mdc-circular-progress__indeterminate-circle-graphic"
            viewBox="0 0 32 32"
            xmlns="http://www.w3.org/2000/svg"
          >
            <circle
              cx="16"
              cy="16"
              r="12.5"
              stroke-dasharray="78.54"
              stroke-dashoffset="39.27"
            />
          </svg>
        `
      },
      large: {
        determinate: html`
          <svg
            class="mdc-circular-progress__determinate-circle-graphic"
            viewBox="0 0 48 48"
            xmlns="http://www.w3.org/2000/svg"
          >
            <circle
              class="mdc-circular-progress__determinate-circle"
              cx="24"
              cy="24"
              r="18"
              stroke-dasharray="113.097"
              stroke-dashoffset="113.097"
            />
          </svg>
        `,
        indeterminate: html`
          <svg
            class="mdc-circular-progress__indeterminate-circle-graphic"
            viewBox="0 0 48 48"
            xmlns="http://www.w3.org/2000/svg"
          >
            <circle
              cx="24"
              cy="24"
              r="18"
              stroke-dasharray="113.097"
              stroke-dashoffset="56.549"
            />
          </svg>
        `
      }
    };
    return svg[size][kind];
  }

  render() {
    const classes = {
      [`${this.theme}`]: true,
      'mdc-circular-progress--small': this.size == 'small',
      'mdc-circular-progress--medium': this.size == 'medium',
      'mdc-circular-progress--large': this.size == 'large'
    };
    // Note formatting might introduce space between divs and product unwanted results
    // prettier-ignore
    // clang-format off
    return html`
<div class="${classMap(classes)} mdc-circular-progress mdc-circular-progress--indeterminate" role="progressbar" aria-label="Example Progress Bar" aria-valuemin="0" aria-valuemax="1">
  <div class="mdc-circular-progress__determinate-container">
    ${this.renderSvg(this.size, 'determinate')}
  </div>
  <div class="mdc-circular-progress__indeterminate-container">
    <div class="mdc-circular-progress__spinner-layer">
      <div class="mdc-circular-progress__circle-clipper mdc-circular-progress__circle-left">
        ${this.renderSvg(this.size, 'indeterminate')}
      </div><div class="mdc-circular-progress__gap-patch">
        ${this.renderSvg(this.size, 'indeterminate')}
      </div><div class="mdc-circular-progress__circle-clipper mdc-circular-progress__circle-right">
        ${this.renderSvg(this.size, 'indeterminate')}
      </div>
    </div>
  </div>
</div>
	  `;
    // prettier-ignore-end
    // clang-format on
  }
}
