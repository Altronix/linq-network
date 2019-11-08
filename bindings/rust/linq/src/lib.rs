pub mod linq;
pub use linq::*;

pub fn running() -> bool {
    unsafe { linq_sys::sys_running() }
}
