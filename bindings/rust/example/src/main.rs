extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    linq::init()
        .listen(PORT)
        .on_heartbeat(move |_l, sid| {
            println!("[HEARTBEAT] {}", sid);
            // l.send(linq::Request::Get("/ATX/about"), sid, |e, json| {
            //     println!("[RESPONSE] {}, {}", e, json);
            // });
        })
        .on_alert(on_alert)
        .on_error(|_l, e, _sid| println!("[ERROR] {}", e))
        .start();
    println!("Listening on port {}", PORT);
}

// Example alert callback with a static method
fn on_alert(_l: &linq::Linq, sid: &str) {
    println!("[ALERT] {}", sid);
}
