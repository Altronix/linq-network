import { Store } from 'redux';
import { RootState } from '../src/store/reducers';
import { RootDispatch } from '../src/store/action';
import { container } from '../src/ioc/container.root';
import { SYMBOLS } from '../src/ioc/constants.root';
export const actions: RootDispatch = container.get(SYMBOLS.ACTION_SERVICE);
export const store: Store<RootState> = container.get(SYMBOLS.STORE_SERVICE);
export default { actions, store };
