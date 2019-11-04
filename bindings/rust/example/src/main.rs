extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    let mut linq = linq::Linq::new();
    let socket = linq.listen(PORT);
    println!("Listening on port {}", PORT);

    // Setup Callbacks with lamda or static function
    linq.on_heartbeat(on_heartbeat)
        .on_alert(|_l, sid| println!("[ALERT] {}", sid))
        .on_error(|_l, e, _sid| println!("[ERROR] {}", e));

    // Main Loop
    while linq::running() {
        if !(linq.poll(200) == 0) {}
    }

    // Clean Up
    println!("GOOD BYE");
    linq.shutdown(socket);
}

// Example alert callback with a static method
fn on_heartbeat(_l: &mut linq::Linq, sid: &str) {
    println!("[HEARTBEAT] {}", sid);
}
