extern crate linq;

use std::thread;
use std::time;

fn main() {
    let mut linq = linq::Linq::new();
    let socket = linq.listen("tcp://*:33455");

    linq.on_heartbeat(|_l, sid| println!("[HEARTBEAT] {}", sid));
    linq.on_alert(|_l, sid| println!("[ALERT] {}", sid));
    linq.on_error(|_l, e, _sid| println!("[ERROR] {}", e));

    loop {
        thread::sleep(time::Duration::from_millis(250));
        if !(linq.poll() == 0) {
            break;
        };
    }

    linq.shutdown(socket);
}
