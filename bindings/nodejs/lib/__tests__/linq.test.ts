import "jest";
import { Method, LinqNetworkConstructorArgs, LinqBinding } from "../types";
import { LinqNetwork } from "../LinqNetwork";
import makeMockBinding from "./__mocks__/binding.mock";

function setup(args?: LinqNetworkConstructorArgs) {
  let binding = makeMockBinding();
  let netw = new LinqNetwork(args, binding);
  return { binding: binding as jest.Mocked<LinqBinding>, network: netw };
}

test("Should run", () => {
  let network = new LinqNetwork();
  expect(network.version().length).toBeTruthy();
});

test("Linq should send", async () => {
  let { binding, network } = setup();
  binding.send.mockReturnValue(new Promise(resolve => resolve('{"mock":42}')));
  await network.send("serial", "GET", "/ATX");
  expect(binding.send).toHaveBeenCalledTimes(1);
  binding.send.mockClear();
});

test("Linq should on", async () => {
  // TODO - figure how to test emitter with a mock
});

test("Linq should listen with number", async () => {
  let { binding, network } = setup();
  network.listen(33);
  expect(binding.listen).toHaveBeenCalledWith("tcp://*:33");
  expect(binding.listen).toHaveBeenCalledTimes(1);
  binding.listen.mockClear();
});

test("Linq should listen with string", async () => {
  let { binding, network } = setup();
  network.listen("tcp://*:33");
  expect(binding.listen).toHaveBeenCalledWith("tcp://*:33");
  expect(binding.listen).toHaveBeenCalledTimes(1);
  binding.listen.mockClear();
});

test("Linq should connect with number", async () => {
  let { binding, network } = setup();
  network.connect(33);
  expect(binding.connect).toHaveBeenCalledWith("tcp://*:33");
  expect(binding.connect).toHaveBeenCalledTimes(1);
  binding.connect.mockClear();
});

test("Linq should connect with string", async () => {
  let { binding, network } = setup();
  network.connect("tcp://*:33");
  expect(binding.connect).toHaveBeenCalledWith("tcp://*:33");
  expect(binding.connect).toHaveBeenCalledTimes(1);
  binding.connect.mockClear();
});

test("Linq should run", async () => {
  jest.useFakeTimers();
  let { binding, network } = setup();
  network.run(30);
  expect(binding.poll).toHaveBeenCalledTimes(0);
  jest.advanceTimersByTime(30);
  expect(binding.poll).toHaveBeenCalledWith(30);
  binding.poll.mockClear();
});

test("Linq should shutdown", async () => {
  jest.useFakeTimers();
  let { binding, network } = setup();
  network.run(30);
  expect(binding.poll).toHaveBeenCalledTimes(0);
  network.shutdown();
  jest.advanceTimersByTime(30);
  expect(binding.poll).toHaveBeenCalledTimes(0);
  binding.poll.mockClear();
});
