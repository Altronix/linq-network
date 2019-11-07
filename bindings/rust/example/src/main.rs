extern crate linq;
extern crate rocket;

static PORT: u32 = 33455;

use std::sync::{Arc, Mutex};

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    let l = Arc::new(Mutex::new(
        linq::init()
            .register(linq::Event::on_heartbeat(move |l, sid| {
                println!("[S] Received HEARTBEAT from [{}]", sid);
                l.send(linq::Request::Get("/ATX/about"), sid, |_e, json| {
                    println!("[S] Received RESPONSE from [{}]\n{}", "TODO", json);
                });
            }))
            .register(linq::Event::on_alert(|_l, sid| {
                println!("[S] Received ALERT from [{}]", sid)
            }))
            .register(linq::Event::on_error(|_l, e, _sid| {
                println!("[S] Received ERROR [{}]", e)
            }))
            .listen(linq::Endpoint::Tcp(PORT)),
    ));

    let t = std::thread::spawn(move || linq::task(Arc::clone(&l)));

    t.join().unwrap();
}
