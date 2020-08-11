export interface Action<T = any> {
  type: T;
}

type ActionPayload<K extends string, TAction extends Action<K>> = Omit<
  TAction,
  'type'
>;
export function actionCreator<
  TAction extends Action<K>,
  K extends string = string
>(type: K) {
  return function(payload?: ActionPayload<K, TAction>) {
    return payload ? { ...payload, type } : { type };
  };
}
