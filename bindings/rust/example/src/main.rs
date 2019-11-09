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
    let mut result = "[Devices]:\n".to_string();
    for (k, v) in linq.lock().unwrap().devices().iter() {
        let next = format!("[SERIAL]: {} [PRODUCT]: {}\n", k, v).to_string();
        result.push_str(&next);
    }
    result
}

#[get("/proxy")]
fn proxy_route(linq: State<LinqDb>) -> String {
    let linq = linq.lock().unwrap();
    linq.send_cb(Request::Get("/ATX/about"), "", move |_e, json| {
        println!("[S] Received RESPONSE from [{}]\n{}", "", json);
    });
    // let linq = linq.lock().unwrap();
    // linq.send(Request::Get("/ATX/about"), "").close();
    "TODO".to_string()
}

#[get("/hello")]
fn hello_route() -> String {
    "hello".to_string()
}

// Initialize Linq
fn linq() -> Linq {
    Linq::new()
        .register(Event::on_heartbeat(move |_linq, id| {
            println!("[S] Received HEARTBEAT from [{}]", id);
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

    // TODO https://github.com/SergioBenitez/Rocket/issues/180
    // Need to 'await' for fix for clean shutdown. (pun intended).
    // rocket(clone).launch();

    t.join().unwrap();
}
