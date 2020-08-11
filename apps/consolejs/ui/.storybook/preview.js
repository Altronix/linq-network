import 'reflect-metadata';
import { html } from 'lit-element';
import { addDecorator } from '@storybook/html';

addDecorator(
  storyFn =>
    `
    <atx-provider>
      ${storyFn()}
    </atx-provider>
  `
);
