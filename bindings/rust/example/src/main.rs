extern crate linq;

fn main() {
    let linq = linq::Linq::new();
    let socket = linq.listen("tcp://*:33455");

    // ...

    linq.shutdown(socket);
}
