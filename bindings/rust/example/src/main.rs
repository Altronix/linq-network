extern crate ctrlc;
extern crate linq;

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;

static PORT: u32 = 33455;

fn main() {
    // Setup Signal Handler
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();
    ctrlc::set_handler(move || r.store(false, Ordering::SeqCst))
        .expect("Error setting ctrlc handler");

    // Setup Linq
    let mut linq = linq::Linq::new();
    let socket = linq.listen(PORT);
    println!("Listening on port {}", PORT);

    // Setup Callbacks
    linq.on_heartbeat(|_l, sid| println!("[HEARTBEAT] {}", sid))
        .on_alert(|_l, sid| println!("[ALERT] {}", sid))
        .on_error(|_l, e, _sid| println!("[ERROR] {}", e));

    // Main Loop
    while running.load(Ordering::SeqCst) {
        if !(linq.poll(200) == 0) {}
    }

    // Clean Up
    println!("GOOD BYE");
    linq.shutdown(socket);
}
