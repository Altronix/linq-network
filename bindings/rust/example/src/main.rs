extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    let mut linq = linq::Linq::new();
    let socket = linq.listen(PORT);
    println!("Listening on port {}", PORT);

    // Setup Callbacks with lamda or static function
    linq.on_heartbeat(move |l, sid| {
        println!("[HEARTBEAT] {}", sid);
        l.send(
            linq::Request::Get("/ATX/about".to_string()),
            sid,
            |e, json| {
                println!("[RESPONSE] {}, {}", e, json);
            },
        );
    })
    .on_alert(on_alert)
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
fn on_alert(_l: &linq::Linq, sid: &str) {
    println!("[HEARTBEAT] {}", sid);
}
