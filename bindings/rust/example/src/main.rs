#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]

#[macro_use]
extern crate rocket;
extern crate linq;

// use rocket::response::content;
use linq::{Endpoint, Event, LinqConnection, Request};
use rocket::State;
use std::sync::{Arc, Mutex};

static PORT: u32 = 33455;

#[get("/linq")]
fn linq_route(linq: State<Arc<Mutex<LinqConnection>>>) -> String {
    linq.lock().unwrap().ctx.device_count().to_string()
}

#[get("/hello")]
fn hello_route() -> String {
    "hello".to_string()
}

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    let linq = linq::init()
        .register(Event::on_heartbeat(move |l, sid| {
            println!("[S] Received HEARTBEAT from [{}]", sid);
            l.send(Request::Get("/ATX/about"), sid, |_e, json| {
                println!("[S] Received RESPONSE from [{}]\n{}", "TODO", json);
            });
        }))
        .register(Event::on_alert(|_l, sid| {
            println!("[S] Received ALERT from [{}]", sid)
        }))
        .register(Event::on_error(|_l, e, _sid| {
            println!("[S] Received ERROR [{}]", e)
        }))
        .listen(Endpoint::Tcp(PORT));

    let linq = Arc::new(Mutex::new(linq));
    let rocket_linq = Arc::clone(&linq);
    let t = std::thread::spawn(move || linq::task(Arc::clone(&linq)));

    rocket::ignite()
        .mount("/", routes![linq_route, hello_route])
        .manage::<Arc<Mutex<LinqConnection>>>(rocket_linq)
        .launch();

    t.join().unwrap();
}
