#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]

#[macro_use]
extern crate rocket;
extern crate linq;

use linq::{Endpoint, Event, Linq, Request};
use rocket::{Rocket, State};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

static PORT: u32 = 33455;

type LinqDb = Arc<Mutex<Linq>>;

#[get("/devices")]
fn linq_route(linq: State<LinqDb>) -> String {
    linq.lock().unwrap().device_count().to_string()
}

#[get("/proxy")]
fn proxy_route(_linq: State<LinqDb>) -> String {
    "TODO".to_string()
}

#[get("/hello")]
fn hello_route() -> String {
    "hello".to_string()
}

// Initialize Linq
fn linq() -> Linq {
    Linq::new()
        .register(Event::on_heartbeat(move |linq, id| {
            println!("[S] Received HEARTBEAT from [{}]", id);
            let sid = id.to_owned();
            linq.send(Request::Get("/ATX/about"), id, move |_e, json| {
                println!("[S] Received RESPONSE from [{}]\n{}", sid, json);
            });
        }))
        .register(Event::on_alert(|_l, sid| {
            println!("[S] Received ALERT from [{}]", sid)
        }))
        .register(Event::on_error(|_l, e, _sid| {
            println!("[S] Received ERROR [{}]", e)
        }))
        .listen(Endpoint::Tcp(PORT))
}

// Initialize Rocket with Linq Context
fn rocket(linq: LinqDb) -> Rocket {
    rocket::ignite()
        .mount("/linq", routes![linq_route, hello_route, proxy_route])
        .manage(linq)
}

fn main() {
    let linq = Arc::new(Mutex::new(linq()));

    let clone = Arc::clone(&linq);
    let t = std::thread::spawn(move || {
        while linq::running() {
            thread::sleep(Duration::from_millis(50));
            let linq = linq.lock().unwrap();
            linq.poll(1);
        }
    });

    rocket(clone).launch();

    t.join().unwrap();
}
