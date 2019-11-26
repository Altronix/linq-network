#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]
#![feature(async_closure)]

#[macro_use]
extern crate rocket;
extern crate rocket_contrib;
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
use std::path::PathBuf;
use std::sync::{Arc, Mutex};
use std::time::Duration;

static PORT: u32 = 33455;

type LinqDb = Arc<Mutex<Context>>;

// Return a JSON map of all the connected devices
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

// Make a GET request to a LinQ connected device
#[get("/proxy/<id>/<path..>")]
async fn proxy_route_get(
    linq: State<'_, LinqDb>,
    id: String,
    path: PathBuf,
) -> content::Json<String> {
    if let Some(path) = path.to_str() {
        let request = Request::Get(&path);
        proxy_request(linq, id, request).await
    } else {
        content::Json("{\"error\":\"bad args\"}".to_string())
    }
}

// Make a POST request to a LinQ connected device
#[post("/proxy/<id>/<path..>", format = "json", data = "<data>")]
async fn proxy_route_post(
    linq: State<'_, LinqDb>,
    id: String,
    path: PathBuf,
    data: String,
) -> content::Json<String> {
    if let Some(path) = path.to_str() {
        let request = Request::Post(&path, &data);
        proxy_request(linq, id, request).await
    } else {
        content::Json("{\"error\":\"bad args\"}".to_string())
    }
}

// Send the request to the device and return the result
async fn proxy_request<'a>(
    linq: State<'_, LinqDb>,
    id: String,
    request: Request<'a>,
) -> content::Json<String> {
    let future = {
        let linq = linq.lock().unwrap();
        linq.send(request, &id)
    };
    match future.await {
        Ok(response) => content::Json(response.json),
        Err(n) => {
            let mut e = "Error: ".to_string();
            e.push_str(&n.to_string());
            content::Json(e)
        }
    }
}

// Initialize Rocket with Linq Context
fn rocket(linq: LinqDb) -> Rocket {
    rocket::ignite()
        .mount(
            "/linq",
            routes![linq_route, proxy_route_get, proxy_route_post],
        )
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
    // start non blocking linq async task
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
