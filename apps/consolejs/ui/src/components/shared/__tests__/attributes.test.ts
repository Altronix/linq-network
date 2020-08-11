import { html, fixture, expect } from '@open-wc/testing';
import { readAttribute, makeAttribute, setAttribute } from '../attributes';

type Sizes = 'small' | 'medium' | 'large';
interface Attributes {
  foo: boolean;
  bar: string;
  size: Sizes;
  loading: boolean;
}

describe('attributes', () => {
  it('should read attributes', () => {
    let el = document.createElement('input');
    setAttribute(el, makeAttribute('foo'));
    setAttribute(el, makeAttribute('bar', 'baz'));
    setAttribute(el, makeAttribute('size', 'small'));
    setAttribute(el, makeAttribute('loading'));
    let foo: boolean | undefined = readAttribute<boolean>(el, 'foo');
    let bar: string | undefined = readAttribute(el, 'bar');
    let size: Sizes | undefined = readAttribute(el, 'size');
    let loading: boolean | undefined = readAttribute<boolean>(el, 'loading');
    expect(foo).to.eq(true);
    expect(bar).to.eq('baz');
    expect(size).to.eq('small');
    expect(loading).to.eq(true);
  });
});
