import { html } from 'lit-element';
import './pagination';
import { BPagination } from './pagination';

export default { title: 'b-pagination' };

export const simple = () => {
  let pagination: BPagination = new BPagination();
  pagination.size = 'small';
  pagination.simple = true;
  pagination.current = 1;
  pagination.total = 87;
  pagination.perPage = 4;
  pagination.buttonsAfter = 3;
  pagination.buttonsBefore = 1;
  pagination.addEventListener(
    'b-prev',
    () => (pagination.current = pagination.current - 1)
  );
  pagination.addEventListener(
    'b-next',
    () => (pagination.current = pagination.current + 1)
  );
  return pagination;
};

export const simpleOneItemPerPage = () => {
  let pagination: BPagination = new BPagination();
  pagination.size = 'small';
  pagination.simple = true;
  pagination.current = 1;
  pagination.total = 10;
  pagination.perPage = 1;
  pagination.buttonsAfter = 3;
  pagination.buttonsBefore = 1;
  pagination.addEventListener(
    'b-prev',
    () => (pagination.current = pagination.current - 1)
  );
  pagination.addEventListener(
    'b-next',
    () => (pagination.current = pagination.current + 1)
  );
  return pagination;
};

export const buttons = () => {
  let pagination: BPagination = new BPagination();
  pagination.size = 'small';
  pagination.current = 1;
  pagination.total = 87;
  pagination.perPage = 1;
  pagination.buttonsAfter = 3;
  pagination.buttonsBefore = 1;
  pagination.addEventListener(
    'b-prev',
    () => (pagination.current = pagination.current - 1)
  );
  pagination.addEventListener(
    'b-next',
    () => (pagination.current = pagination.current + 1)
  );
  return pagination;
};
