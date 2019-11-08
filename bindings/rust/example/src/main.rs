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

struct Counter {
    x: u32,
}

#[get("/counter")]
fn counter(c: State<Mutex<Counter>>) -> String {
    let mut c = c.lock().unwrap();
    c.x.to_string()
}

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    let conn = linq::init()
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

    let conn = Arc::new(Mutex::new(conn));
    let rocket_linq = Arc::clone(&conn);
    let t = std::thread::spawn(move || linq::task(Arc::clone(&conn)));

    rocket::ignite()
        .mount("/", routes![linq_route, counter])
        .manage::<Arc<Mutex<LinqConnection>>>(rocket_linq)
        .manage(Mutex::new(Counter { x: 0 }))
        .launch();

    // t.join().unwrap();
}
