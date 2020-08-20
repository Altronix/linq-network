// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

mod event;
mod simple_future;

pub mod arc;
pub mod error;
pub mod polling;

pub use event::{Event, EventStream};
pub use polling::{netw_socket, Endpoint, Request};
