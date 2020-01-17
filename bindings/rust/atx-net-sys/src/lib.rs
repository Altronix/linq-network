// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn linq_allocators() {
        unsafe {
            let mut l = linq_create(std::ptr::null_mut(), std::ptr::null_mut());
            linq_destroy(&mut l);
        }
    }
}
