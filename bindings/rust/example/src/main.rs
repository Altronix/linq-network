#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]

#[macro_use]
extern crate rocket;
extern crate linq;

// use rocket::response::content;
use linq::{Endpoint, Event, Linq, Request};
use rocket::State;
use std::sync::{Arc, Mutex};

static PORT: u32 = 33455;

#[get("/linq")]
fn linq_route(linq: State<Arc<Mutex<Linq>>>) -> String {
    linq.lock().unwrap().device_count().to_string()
}

#[get("/hello")]
fn hello_route() -> String {
    "hello".to_string()
}

fn main() {
    // Setup Linq
    println!("Listening on port {}", PORT);
    let linq = Arc::new(Mutex::new(
        Linq::new()
            .register(Event::on_heartbeat(move |l, device_id| {
                println!("[S] Received HEARTBEAT from [{}]", device_id);
                let sid = device_id.to_owned();
                l.send(Request::Get("/ATX/about"), device_id, move |_e, json| {
                    println!("[S] Received RESPONSE from [{}]\n{}", sid, json);
                });
            }))
            .register(Event::on_alert(|_l, sid| {
                println!("[S] Received ALERT from [{}]", sid)
            }))
            .register(Event::on_error(|_l, e, _sid| {
                println!("[S] Received ERROR [{}]", e)
            }))
            .listen(Endpoint::Tcp(PORT)),
    ));

    let linq_spawn = Arc::clone(&linq);
    let rocket_spawn = Arc::clone(&linq);
    let t = std::thread::spawn(move || {
        while linq::running() {
            let linq_spawn = linq_spawn.lock().unwrap();
            linq_spawn.poll(1);
        }
    });

    // TODO figure out close properly
    rocket::ignite()
        .mount("/", routes![linq_route, hello_route])
        .manage(rocket_spawn)
        .launch();

    t.join().unwrap();
}
