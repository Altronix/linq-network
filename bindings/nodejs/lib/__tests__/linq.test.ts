import "jest";
import { network } from "../";

test("Should run", () => {
  expect(network.version().length).toBeTruthy();
});
