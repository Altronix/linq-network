extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    linq::init()
        .register(linq::Event::on_heartbeat(|_l, _sid| println!("[H 0")))
        .register(linq::Event::on_heartbeat(|_l, _sid| println!("[H 1")))
        .register(linq::Event::on_heartbeat(|_l, _sid| println!("[H 2")))
        .on_heartbeat(move |l, sid| {
            println!("[HEARTBEAT] {}", sid);
            l.send(linq::Request::Get("/ATX/about"), sid, |e, json| {
                println!("[RESPONSE] {}, {}", e, json);
            });
        })
        .on_alert(|_l, sid| println!("[ALERT] {}", sid))
        .on_error(|_l, e, _sid| println!("[ERROR] {}", e))
        .listen(PORT)
        .start();
}
