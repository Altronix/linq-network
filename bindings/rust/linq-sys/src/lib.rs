#![allow(non_camel_case_types)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);

        unsafe {
            let l = linq_create(std::ptr::null_mut(), std::ptr::null_mut());
        }
    }
}
