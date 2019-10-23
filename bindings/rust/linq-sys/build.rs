extern crate bindgen;
extern crate cmake;

use std::env;
use std::path::PathBuf;

fn main() {
    // Build linq TODO build static
    let dst = cmake::build("../../../");
    let out = dst.display();

    // Add compiler flags
    println!("cargo:rustc-link-search=native={}/lib", out);
    println!("cargo:rustc-link-search=native={}/build/install/lib", out);
    println!("cargo:rustc-link-search=native={}/build/install/lib64", out);
    println!("cargo:rustc-link-lib=static=linq");
    println!("cargo:rustc-link-lib=static=zmq");
    println!("cargo:rustc-link-lib=static=czmq");
    println!("cargo:rustc-link-lib=uuid");
    println!("cargo:rustc-link-lib=stdc++");
    println!("cargo:rustc-link-lib=m");
    println!("cargo:rustc-link-lib=rt");

    // Generate bindings
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .generate()
        .expect("Unable to generate bindings");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings");
}
