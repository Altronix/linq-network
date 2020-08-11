import { html, TemplateResult } from 'lit-element';
import { BModal } from './modal';
import './modal';

export default { title: 'b-modal' };

export const basic = () => {
  let root = document.createElement('div');
  let modal: BModal = document.createElement('b-modal') as BModal;
  let button = document.createElement('button');
  let content = document.createElement('div');
  content.innerHTML = '<h1>Test</h1><p>Hello World</p>';
  button.innerText = 'Click me';
  button.addEventListener('click', () => (modal.show = true));
  modal.addEventListener('b-close', () => (modal.show = false));
  modal.appendChild(content);
  root.appendChild(modal);
  root.appendChild(button);
  return root;
};
