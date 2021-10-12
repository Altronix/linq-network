import { network as linq } from "../../../../";

linq
  .listen("tcp://*:33455")
  .tick(50)
  .events()
  .subscribe((event) => {
    console.log(event);
  });
