import { Binding } from "../../types";
import { Mock } from "./binding";
jest.mock("./binding");

export default function(): Binding {
  return new Mock() as Binding;
}
