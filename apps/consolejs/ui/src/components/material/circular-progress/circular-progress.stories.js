import './circular-progress';

export default { title: 'circular-progress' };

export const small = () => {
  const el = document.createElement('m-circular-progress');
  el.size = 'small';
  return el;
};

export const medium = () => {
  const el = document.createElement('m-circular-progress');
  el.size = 'medium';
  return el;
};

export const large = () => {
  const el = document.createElement('m-circular-progress');
  el.size = 'large';
  return el;
};
