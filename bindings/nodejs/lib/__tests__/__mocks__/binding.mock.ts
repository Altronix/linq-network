import { LinqBinding } from "../../types";
import { Mock } from "./binding";
jest.mock("./binding");

export default function(): LinqBinding {
  return new Mock() as LinqBinding;
}
