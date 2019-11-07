extern crate linq;

static PORT: u32 = 33455;

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    linq::init()
        .register(linq::Event::on_heartbeat(move |l, sid| {
            println!("[HEARTBEAT] {}", sid);
            l.send(linq::Request::Get("/ATX/about"), sid, |e, json| {
                println!("[RESPONSE] {}, {}", e, json);
            });
        }))
        .register(linq::Event::on_alert(|_l, sid| println!("[ALERT] {}", sid)))
        .register(linq::Event::on_error(|_l, e, _sid| {
            println!("[ERROR] {}", e)
        }))
        .listen(PORT)
        .start();
}
