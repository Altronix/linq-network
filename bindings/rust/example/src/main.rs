#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]

#[macro_use]
extern crate rocket;
#[macro_use]
extern crate rocket_contrib;
#[macro_use]
extern crate serde_derive;

extern crate linq;

use futures::executor::block_on;
use linq::{Endpoint, Event, Linq, Request};
use rocket::{Rocket, State};
use rocket_contrib::json::{Json, JsonValue};
use std::option::Option;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

static PORT: u32 = 33455;
type ID = String;

type LinqDb = Arc<Mutex<Linq>>;

#[get("/devices")]
fn linq_route(linq: State<LinqDb>) -> String {
    // TODO send JSON
    let mut result = "[Devices]:\n".to_string();
    for (k, v) in linq.lock().unwrap().devices().iter() {
        let next = format!("[SERIAL]: {} [PRODUCT]: {}", k, v).to_string();
        result.push_str(&next);
    }
    result
}

#[derive(Serialize, Deserialize)]
struct ProxyRequest {
    meth: String,
    path: String,
    data: Option<String>,
}

#[post("/proxy/<id>", format = "json", data = "<data>")]
fn proxy_route(
    linq: State<LinqDb>,
    id: ID,
    data: Json<ProxyRequest>,
) -> String {
    // Check ProxyRequest params and store request
    let request = match data.0.meth.as_str() {
        "POST" => match &data.0.data {
            Some(d) => Some(Request::Post(&data.0.path, d.as_str())),
            _ => None,
        },
        "DELETE" => Some(Request::Delete(&data.0.path)),
        _ => Some(Request::Get(data.0.path.as_ref())),
    };
    // If request was valid make request
    if let Some(request) = request {
        let f;
        {
            // NOTE - Do not call "block_on()" while holding the mutex
            // TODO - rocket.rs should accept a future eventually - and
            // therefore we do not need to call block_on() in the route
            // https://github.com/SergioBenitez/rocket/issues/1065
            let linq = linq.lock().unwrap();
            f = linq.send(request, id.as_str());
        }
        match block_on(f) {
            Ok(s) => s,
            Err(n) => {
                let mut e = "Error: ".to_string();
                e.push_str(&n.to_string());
                e
            }
        }
    } else {
        "BAD ARGS".to_string()
    }
}

#[get("/hello")]
fn hello_route() -> String {
    "hello".to_string()
}

// Initialize Linq
fn linq() -> Linq {
    Linq::new()
        .register(Event::on_heartbeat(move |_linq, id| {
            // NOTE cannot block inside the callbacks because they share the task with
            // linq.poll() TODO switch to "stream" api for events (instead of callbacks)
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
    let linq = Arc::new(Mutex::new(linq().pin()));

    let clone = Arc::clone(&linq);
    let t = std::thread::spawn(move || {
        while linq::running() {
            thread::sleep(Duration::from_millis(50));
            let linq = linq.lock().unwrap();
            linq.poll(1);
        }
    });

    let _r = std::thread::spawn(move || rocket(clone).launch());

    t.join().unwrap();
    // TODO https://github.com/SergioBenitez/Rocket/issues/180
    // Need to 'await' for fix for clean shutdown. (pun intended).
    // r.join().unwrap();
}
