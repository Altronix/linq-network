import { TemplateResult } from 'lit-element';
export type Sizes = 'small' | 'medium' | 'large' | 'normal';
export type Colors = 'primary' | 'info' | 'succes' | 'warning' | 'danger';
export type IconWhere = 'left' | 'right';
export type NavbarWhere = 'left' | 'right' | 'brand';
export type TextFieldType =
  | 'text'
  | 'search'
  | 'tel'
  | 'url'
  | 'email'
  | 'password'
  | 'date'
  | 'month'
  | 'week'
  | 'time'
  | 'datetime-local'
  | 'number'
  | 'color';
export const NODE_TYPES = { ELEMENT: 1, TEXT: 3 };
export type TableVariants = 'basic' | 'crud';
export type ExtendedTableData<T> = T & TableDataVarientOptions;
export interface TableDataVarientOptions {
  idx?: number;
  actions?: TemplateResult;
}
export type Positions =
  | 'left'
  | 'right'
  | 'top'
  | 'bottom'
  | 'top-left'
  | 'top-right'
  | 'bottom-left'
  | 'bottom-right';
