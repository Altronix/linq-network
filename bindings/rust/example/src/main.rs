extern crate linq;

use std::sync::{Arc, Mutex};
use std::thread;
static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    let mut linq = linq::init();
    linq.listen(PORT);
    println!("Listening on port {}", PORT);

    let linq = Arc::new(Mutex::new(linq));
    {
        let mut linq = linq.lock().unwrap();

        // Setup Callbacks with lamda or static function
        linq.on_heartbeat(move |l, sid| {
            println!("[HEARTBEAT] {}", sid);
            l.send(linq::Request::Get("/ATX/about"), sid, |e, json| {
                println!("[RESPONSE] {}, {}", e, json);
            });
        })
        .on_alert(on_alert)
        .on_error(|_l, e, _sid| println!("[ERROR] {}", e));
    }

    let linq_thread;
    {
        let l = Arc::clone(&linq);
        linq_thread = thread::spawn(move || {
            while linq::running() {
                let l = l.lock().unwrap();
                if !(l.poll(200) == 0) {}
            }
            println!("GOOD BYE");
        });
        // Main Loop
    }

    // Clean Up
    linq_thread.join().unwrap();
}

// Example alert callback with a static method
fn on_alert(_l: &linq::Linq, sid: &str) {
    println!("[ALERT] {}", sid);
}
