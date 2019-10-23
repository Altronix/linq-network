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
