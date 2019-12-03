// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

extern crate bindgen;
extern crate cmake;

use std::env;
use std::fs;
use std::path::PathBuf;

// TODO - Sometimes we are built as a submodule to another project, Sometimes
// we are built from git root. Should find better way to build project...
fn find_cmake_list() -> String {
    let f = fs::metadata("../../../../CMakeLists.txt");
    match f {
        Ok(_) => "../../../../".to_owned(),
        _ => "../../../".to_owned(),
    }
}

fn main() {
    // Build linq-io TODO build static
    let dst = cmake::build(find_cmake_list());
    let out = dst.display();

    // Add compiler flags
    println!("cargo:rustc-link-search=native={}/lib", out);
    println!("cargo:rustc-link-search=native={}/build/install/lib", out);
    println!("cargo:rustc-link-search=native={}/build/install/lib64", out);
    println!("cargo:rustc-link-lib=static=linq-netw");
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
