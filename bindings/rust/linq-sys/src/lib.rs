#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);

        unsafe {
            let mut l = linq_create(std::ptr::null_mut(), std::ptr::null_mut());
            linq_destroy(&mut l);
        }
    }
}
