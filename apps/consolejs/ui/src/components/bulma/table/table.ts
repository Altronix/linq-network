import {
  LitElement,
  customElement,
  html,
  property,
  TemplateResult
} from 'lit-element';
import {
  Colors,
  Sizes,
  TableVariants,
  TableDataVarientOptions,
  NavbarWhere
} from '../bulma-types';
import { readAttribute, writeAttribute } from '../../shared/attributes';
import { classMap } from 'lit-html/directives/class-map';
import { styles } from '../styles';

import * as scss from './table.styles.scss';

export interface TableColumnData<T extends TableDataVarientOptions> {
  label: keyof T;
  field?: string;
  key?: keyof T;
  hide?: boolean;
  width?: number | string;
  numeric?: boolean;
  centered?: boolean;
  sortable?: boolean;
  sort?: <A, B>(a: A, b: B) => boolean;
  sticky?: boolean;
  ['header-class']?: string;
  ['cell-class']?: string;
}

export interface Table<T> {
  data: T[];
  columns: TableColumnData<T>[];
}

type TableDataHtml<T> = {
  [key in keyof T]: TemplateResult;
};

@customElement('b-table')
export class BTable<T> extends LitElement {
  static styles = styles(scss.toString());
  @property({ type: Boolean }) bordered: boolean = false;
  @property({ type: Boolean }) striped: boolean = false;
  @property({ type: Boolean }) footer: boolean = false;
  @property({ type: Boolean }) narrow: boolean = false;
  @property({ type: Boolean }) hoverable: boolean = false;
  @property({ type: Boolean }) pagination: boolean = false;
  @property({ type: Boolean }) checkboxes: boolean = false;
  @property({ type: Boolean }) fullwidth: boolean = false;
  @property({ type: Boolean }) numbered: boolean = false;
  @property({ type: Number }) selected: number = -1;
  @property({ type: String }) variant: TableVariants = 'basic';

  _data: Table<T & TableDataVarientOptions> = {
    data: [],
    columns: []
  };
  set data(data: Table<T & TableDataVarientOptions>) {
    if (this.numbered) {
      data.columns.unshift({ label: 'idx', numeric: true });
      data.data = data.data.map((d, idx) => {
        return { ...d, idx: idx + 1 };
      });
    }
    if (this.variant === 'crud') {
      data.columns.push({ label: 'actions' });
      data.data = data.data.map(d => {
        d['actions'] = html`
          <td>
            <span class="is-hidden">align</span>
            <b-icon size="small" color="info">edit</b-icon>
            <b-icon size="small" color="danger">delete</b-icon>
          </td>
        `;
        return d;
      });
    }
    this._data = data;
    this.requestUpdate();
  }
  get data(): Table<T & TableDataVarientOptions> {
    return this._data;
  }

  renderHeader() {
    return this._data.columns.map(
      c =>
        html`
          <th class="${c.numeric ? 'is-numeric' : ''}">${c.label}</th>
        `
    );
  }

  render() {
    const classes = {
      table: {
        table: true,
        ['is-bordered']: !!this.bordered,
        ['is-striped']: !!this.striped,
        ['is-narrow']: !!this.narrow,
        ['is-hoverable']: !!this.hoverable,
        ['is-fullwidth']: !!this.fullwidth
      },
      column: (c: TableColumnData<T>) => {
        return {
          'is-numeric': !!c.numeric
        };
      },
      row: (idx: number) => {
        return {
          'is-selected': idx === this.selected
        };
      }
    };
    return html`
      <div class="table-container">
        <table class="${classMap(classes.table)}">
          <thead>
            <tr>
              ${this.renderHeader()}
            </tr>
          </thead>
          ${this.footer
            ? html`
                <tfoot>
                  <tr>
                    ${this.renderHeader()}
                  </tr>
                </tfoot>
              `
            : ''}
          <tbody>
            ${this._data.data.map(
              (d, idx) =>
                html`
                  <tr class="${classMap(classes.row(idx))}">
                    ${this._data.columns.map(c =>
                      d[c.label] instanceof TemplateResult
                        ? d[c.label]
                        : html`
                            <td class="${classMap(classes.column(c))}">
                              ${d[c.label]}
                            </td>
                          `
                    )}
                  </tr>
                `
            )}
          </tbody>
        </table>
      </div>
    `;
  }
}
