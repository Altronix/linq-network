import { network as linq } from "../../../../build/install/lib";

linq
  .listen("tcp://*:33455")
  .tick(50)
  .watch()
  .subscribe((event) => {
    console.log(event);
  });