import { Unsubscribe, Store } from 'redux';
import { LitElement } from 'lit-element';
import { SYMBOLS } from '../ioc/constants.root';
import { domInject, domConsumer } from '../components/shared/decorators';
import { decorate } from 'inversify';
import { RootState } from '../store/reducers';
import { actions } from '../store/action';
type Constructor<T> = new (...args: any[]) => T;

export function connect<
  T extends Constructor<LitElement>,
  S = RootState,
  A = typeof actions
>(target: T) {
  @domConsumer()
  class C extends target {
    @domInject(SYMBOLS.ACTION_SERVICE) actions!: A;
    @domInject(SYMBOLS.STORE_SERVICE) store!: Store<S>;
    _storeUnsubscribe!: Unsubscribe;
    connectedCallback() {
      super.connectedCallback();
      this._storeUnsubscribe = this.store.subscribe(() => {
        this.stateChanged(this.store.getState());
      });
      this.stateChanged(this.store.getState());
    }
    disconnectedCallback() {
      this._storeUnsubscribe();
      super.disconnectedCallback();
    }

    stateChanged(s: S) {}
  }
  return C;
}
