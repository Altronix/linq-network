import { html, fixture, expect, oneEvent } from '@open-wc/testing';
import { Container } from 'inversify';
import { LitElement, customElement } from 'lit-element';
import {
  makeDecorators,
  DomInjectEvent,
  DomPropertyMetadata,
  domProvider,
  domConsumer,
  domInject,
  METADATA_KEYS
} from '../decorators';
import { ServiceIdentifier } from '../types';

describe('lazyInject ', () => {
  it('should inject a service', () => {
    const TEST_SERVICE_A = Symbol.for('TestServiceA');
    const TEST_SERVICE_B = Symbol.for('TestServiceB');
    const TEST_SERVICE_C = Symbol.for('TestServiceC');
    const container = new Container();
    const { lazyInject, bind } = makeDecorators(container);

    interface Service {
      name: string;
    }

    @bind(TEST_SERVICE_A)
    class ServiceA implements Service {
      name: string = 'ServiceA';
    }
    @bind(TEST_SERVICE_B)
    class ServiceB implements Service {
      name: string = 'ServiceB';
    }
    @bind(TEST_SERVICE_C)
    class ServiceC implements Service {
      name: string = 'ServiceC';
    }

    class Foo {
      @lazyInject(TEST_SERVICE_A)
      serviceA!: Service;
      @lazyInject(TEST_SERVICE_B)
      serviceB!: Service;
      @lazyInject(TEST_SERVICE_C)
      serviceC!: Service;
    }
    let foo = new Foo();
    expect(foo.serviceA.name).to.eq('ServiceA');
    expect(foo.serviceB.name).to.eq('ServiceB');
    expect(foo.serviceC.name).to.eq('ServiceC');
  });
});

describe('domInject', () => {
  it('domInject should add metadata', () => {
    const container = new Container();
    const { bind } = makeDecorators(container);
    const TEST_SERVICE_A = Symbol.for('TestServiceA');
    const TEST_SERVICE_B = Symbol.for('TestServiceB');
    const TEST_SERVICE_C = Symbol.for('TestServiceC');
    interface Service {
      name: string;
    }

    class Foo {
      @domInject(TEST_SERVICE_A)
      serviceA!: Service;
      @domInject(TEST_SERVICE_B)
      serviceB!: Service;
      @domInject(TEST_SERVICE_C)
      serviceC!: Service;
    }

    let foo = new Foo();
    let meta: DomPropertyMetadata[] = Reflect.getMetadata(
      METADATA_KEYS.domConsumer,
      foo.constructor
    );

    expect(meta.length).to.eq(3);
    expect(meta[0].id).to.eq(TEST_SERVICE_A);
    expect(meta[0].key).to.eq('serviceA');
    expect(meta[0].target).to.eq(Foo.prototype);
    expect(meta[1].id).to.eq(TEST_SERVICE_B);
    expect(meta[1].key).to.eq('serviceB');
    expect(meta[1].target).to.eq(Foo.prototype);
    expect(meta[2].id).to.eq(TEST_SERVICE_C);
    expect(meta[2].key).to.eq('serviceC');
    expect(meta[2].target).to.eq(Foo.prototype);
  });
});

describe('domConsumer and domProvider', () => {
  it('domProvider should provide for domConsumer', async () => {
    let container = new Container();
    const { bind } = makeDecorators(container);
    const TEST_SERVICE_A = Symbol.for('TestServiceA');
    const TEST_SERVICE_B = Symbol.for('TestServiceB');
    const TEST_SERVICE_C = Symbol.for('TestServiceC');
    interface Service {
      name: string;
    }
    @bind(TEST_SERVICE_A)
    class ServiceA implements Service {
      name: string = 'ServiceA';
    }
    @bind(TEST_SERVICE_B)
    class ServiceB implements Service {
      name: string = 'ServiceB';
    }
    @bind(TEST_SERVICE_C)
    class ServiceC implements Service {
      name: string = 'ServiceC';
    }

    // Provider
    @domProvider('foo-provider', container)
    class FooProvider extends LitElement {
      render() {
        return html`
          <slot></slot>
        `;
      }
    }

    // Wrapper (test events bubble up through shadow DOM)
    @customElement('foo-wrapper')
    class FooMiddle extends LitElement {
      render() {
        return html`
          <slot></slot>
        `;
      }
    }

    // Consumer
    @domConsumer('foo-consumer')
    class Foo extends LitElement {
      @domInject(TEST_SERVICE_A)
      serviceA!: Service;
      @domInject(TEST_SERVICE_B)
      serviceB!: Service;
      @domInject(TEST_SERVICE_C)
      serviceC!: Service;
    }
    let test = await fixture(
      `<foo-provider>
        <foo-wrapper>
          <foo-consumer></foo-consumer>
        </foo-wrapper>
      </foo-provider>`
    );
    let foo: Foo | null = test.querySelector('foo-consumer');
    expect(foo).instanceof(Foo);
    if (foo) {
      expect(foo.serviceA.name).to.eq('ServiceA');
      expect(foo.serviceB.name).to.eq('ServiceB');
      expect(foo.serviceC.name).to.eq('ServiceC');
    }
  });
});
