extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    let mut linq = linq::Linq::new();
    let socket = linq.listen(PORT);
    println!("Listening on port {}", PORT);

    // Setup Callbacks
    linq.on_heartbeat(|_l, sid| println!("[HEARTBEAT] {}", sid))
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
