#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]
#![feature(async_closure)]

#[macro_use]
extern crate rocket;
extern crate rocket_contrib;
#[macro_use]
extern crate serde_derive;

extern crate futures_timer;
extern crate linq_netw;

use futures::executor::block_on;
use futures::future::join;
use futures::prelude::*;
use futures::stream::StreamExt;
use linq_netw::{Context, Endpoint, Event, Request};
use rocket::response::content;
use rocket::{Rocket, State};
use rocket_contrib::json::Json;
use std::option::Option;
use std::sync::{Arc, Mutex};
use std::time::Duration;

static PORT: u32 = 33455;
type ID = String;

type LinqDb = Arc<Mutex<Context>>;

#[get("/devices")]
fn linq_route(linq: State<LinqDb>) -> content::Json<String> {
    let mut ret = "{".to_string();
    let linq = linq.lock().unwrap();
    let sz = linq.devices().len();
    let mut n = 0;
    for (k, v) in linq.devices().iter() {
        let end = if n < sz - 1 { "," } else { "" };
        n = n + 1;
        ret.push_str(&format!("\"{}\":{{\"product\":\"{}\"}}{}", k, v, end));
    }
    ret.push_str("}");
    content::Json(ret)
}

#[derive(Serialize, Deserialize)]
struct ProxyRequest {
    meth: String,
    path: String,
    data: Option<String>,
}

fn parse_request<'a>(data: &'a Json<ProxyRequest>) -> Option<Request<'a>> {
    match data.0.meth.as_str() {
        "POST" => match &data.0.data {
            Some(d) => Some(Request::Post(&data.0.path, d.as_str())),
            _ => None,
        },
        "DELETE" => Some(Request::Delete(&data.0.path)),
        _ => Some(Request::Get(data.0.path.as_ref())),
    }
}

#[post("/proxy/<id>", format = "json", data = "<data>")]
async fn proxy_route(
    linq: State<'_, LinqDb>,
    id: ID,
    data: Json<ProxyRequest>,
) -> content::Json<String> {
    // If request was valid make request
    if let Some(request) = parse_request(&data) {
        let f = {
            let linq = linq.lock().unwrap();
            linq.send(request, &id)
        };
        let response = f.await;
        match response {
            Ok(response) => content::Json(response.json),
            Err(n) => {
                let mut e = "Error: ".to_string();
                e.push_str(&n.to_string());
                content::Json(e)
            }
        }
    } else {
        content::Json("{\"error\":\"bad args\"}".to_string())
    }
}

#[get("/hello")]
fn hello_route() -> String {
    "hello".to_string()
}

// Initialize Rocket with Linq Context
fn rocket(linq: LinqDb) -> Rocket {
    rocket::ignite()
        .mount("/linq", routes![linq_route, hello_route, proxy_route])
        .manage(linq)
}

fn main() -> Result<(), rocket::error::Error> {
    // Create LinQ instance
    let linq = Context::new();

    // Listen to TCP endpoint tcp://*:PORT
    linq.listen(Endpoint::Tcp(PORT));

    // Create a stream to listen to events
    let events = linq
        .events()
        .take_while(|e| future::ready(*e != Event::Ctrlc))
        .for_each(async move |e| {
            match e {
                Event::Heartbeat(s) => println!("[RECEIVED HEARTBEAT] {}", s),
                Event::Alert(s) => println!("[RECEIVED ALERT] {}", s),
                Event::Error(_, _) => println!("[RECEIVED ERROR]"),
                _ => (),
            };
        });

    // Prepare linq with mutex for seperate thread
    let linq = Arc::new(Mutex::new(linq));
    let clone = Arc::clone(&linq);
    let linq_poller = async_std::task::spawn(async move {
        while linq_netw::running() {
            futures_timer::Delay::new(Duration::from_millis(50)).await;
            let linq = linq.lock().unwrap();
            linq.poll(0);
        }
    });

    // Start web server
    let rocket = rocket(clone);
    let shutdown_handle = rocket.get_shutdown_handle();
    let r = std::thread::spawn(move || rocket.launch());

    block_on(join(events, linq_poller));
    shutdown_handle.shutdown();
    r.join().unwrap()?;
    Ok(())
}
