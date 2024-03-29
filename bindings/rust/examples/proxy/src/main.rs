// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#![feature(proc_macro_hygiene)]
#![feature(decl_macro)]
#![feature(async_closure)]

#[macro_use]
extern crate rocket;
extern crate rocket_contrib;
extern crate serde_derive;

extern crate futures_timer;
extern crate linq_network;

use futures::executor::block_on;
use futures::prelude::*;
use futures::stream::StreamExt;
use linq_network::{Endpoint, Event, Request};
use rocket::http::Status;
use rocket::response::content;
use rocket::{Rocket, State};
use std::path::PathBuf;
use std::sync::{Arc, Mutex};

static PORT: u32 = 33455;

type LinqDb = Arc<Mutex<linq_network::arc::Context>>;

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
) -> Result<content::Json<String>, Status> {
    if let Some(path) = path.to_str() {
        let request = Request::Get(&path);
        proxy_request(linq, id, request).await
    } else {
        Err(Status::new(400, ""))
    }
}

// Make a POST request to a LinQ connected device
#[post("/proxy/<id>/<path..>", format = "json", data = "<data>")]
async fn proxy_route_post(
    linq: State<'_, LinqDb>,
    id: String,
    path: PathBuf,
    data: String,
) -> Result<content::Json<String>, Status> {
    if let Some(path) = path.to_str() {
        let request = Request::Post(&path, &data);
        proxy_request(linq, id, request).await
    } else {
        Err(Status::new(400, ""))
    }
}

// Send the request to the device and return the result
async fn proxy_request<'a>(
    linq: State<'_, LinqDb>,
    id: String,
    request: Request<'a>,
) -> Result<content::Json<String>, Status> {
    let future = {
        let linq = linq.lock().unwrap();
        linq.send(request, &id)
    };
    match future.await {
        Ok(response) => match response.result {
            Ok(v) => Ok(content::Json(v)),
            Err(e) => Err(Status::new(e.to_http(), "")),
        },
        Err(_n) => Err(Status::new(400, "")),
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
    let linq = linq_network::arc::Context::new();

    // Listen to TCP endpoint tcp://*:PORT
    linq.listen(Endpoint::Tcp(PORT));

    // Create a stream to listen to events
    let events = linq
        .events()
        .take_while(|e| future::ready(*e != Event::Ctrlc))
        .for_each(async move |e| {
            match e {
                Event::Heartbeat(_serial) => (),
                Event::Alert(_serial) => (),
                Event::Error(_, _) => (),
                _ => (),
            };
        });

    // Start web server
    let rocket = rocket(Arc::new(Mutex::new(linq)));
    let shutdown_handle = rocket.get_shutdown_handle();
    let r = std::thread::spawn(move || rocket.launch());

    block_on(events);
    shutdown_handle.shutdown();
    r.join().unwrap()?;
    Ok(())
}
