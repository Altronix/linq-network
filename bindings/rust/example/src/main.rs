#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]

#[macro_use]
extern crate rocket;
extern crate linq;

// use rocket::response::content;
use rocket::State;
use std::sync::{Arc, Mutex};

static PORT: u32 = 33455;

#[get("/linq")]
fn linq_route(linq: State<Mutex<linq::LinqConnection>>) -> String {
    linq.lock().unwrap().ctx.device_count().to_string()
}

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    let conn = linq::init()
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
        .listen(linq::Endpoint::Tcp(PORT));

    // let linq = Arc::new(Mutex::new(linq));
    // let rocket_linq = Arc::clone(&linq);
    // let t = std::thread::spawn(move || linq::task(Arc::clone(&linq)));

    rocket::ignite()
        .mount("/", routes![linq_route])
        .manage(Mutex::new(conn))
        .launch();

    // t.join().unwrap();
}
