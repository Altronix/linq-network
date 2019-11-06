extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    linq::init()
        .listen(PORT)
        .on_heartbeat(move |l, sid| {
            println!("[HEARTBEAT] {}", sid);
            l.send(linq::Request::Get("/ATX/about"), sid, |e, json| {
                println!("[RESPONSE] {}, {}", e, json);
            });
        })
        .on_alert(|_l, sid| println!("[ALERT] {}", sid))
        .on_error(|_l, e, _sid| println!("[ERROR] {}", e))
        .start();
    println!("Listening on port {}", PORT);
}
