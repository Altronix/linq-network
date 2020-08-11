export function makeAttribute(key: string, value?: string) {
  let attr = document.createAttribute(key);
  if (value) attr.value = value;
  return attr;
}

export function readAttribute<Ret = string, T extends Element = Element>(
  input: T,
  key: string
): Ret | undefined {
  let attr = input.attributes.getNamedItem(key as string);
  return attr
    ? attr.value
      ? ((attr.value as any) as Ret)
      : ((true as any) as Ret)
    : undefined;
}

export function writeAttribute<T extends Element>(
  el: T,
  key: string,
  val?: string
) {
  el.attributes.setNamedItem(makeAttribute(key, val));
}

export function setAttribute<T extends Element>(el: T, attr: Attr) {
  el.attributes.setNamedItem(attr);
}
