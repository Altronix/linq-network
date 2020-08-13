// Copyright (c) 2019-2020 Altronix Corp
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

extern crate bindgen;
extern crate cmake;

// NOTE https://github.com/rust-lang/rust-bindgen/blob/master/book/src/requirements.md
// NOTE For support of mingw and visual studio 2019+ see reference for travis gotcha's here:
//      https://github.com/zauonlok/renderer/blob/master/.github/workflows/cmake-ci.yml

use std::env;
use std::fs;
use std::path::PathBuf;

// TODO - Sometimes we are built as a submodule to another project, Sometimes
// we are built from git root. Should find better way to build project...
fn find_root() -> String {
    let f = fs::metadata("../../../../CMakeLists.txt");
    match f {
        Ok(_) => "../../../../".to_owned(),
        _ => "../../../".to_owned(),
    }
}

// TODO - Find openssl
// for reference https://github.com/sfackler/rust-openssl/blob/master/openssl-sys

fn gen_common_headers(root: &str) {
    let header = format!("{}/libnetwork/netw.h", root);
    let header = fs::read(header).unwrap();
    fs::write("./wrapper.h", header).unwrap();
    let header = format!("{}/libcommon/common.h", root);
    let header = fs::read(header).unwrap();
    fs::write("./common.h", header).unwrap();
}

fn print_windows(out: &std::path::Display<'_>) {
    let root = find_root();
    let header = format!("{}/libcommon/sys/win/sys.h", root);
    let header = fs::read(header).unwrap();
    fs::write("./sys.h", header).unwrap();
    gen_common_headers(&root);
    // Parse the name of libzmq library and remove the extention.
    let libzmq = format!("{}/build/libzmq-static-loc.txt", out);
    let libzmq = fs::read(libzmq).unwrap();
    let libzmq = String::from_utf8(libzmq).unwrap();
    let libzmq = libzmq.split(".").collect::<Vec<&str>>()[0];
    println!("cargo:rustc-link-search=native={}/lib", out);
    println!("cargo:rustc-link-lib=static=linqnetwork");
    println!("cargo:rustc-link-lib=static=linqcommon");
    println!("cargo:rustc-link-lib=static=libczmq");
    println!("cargo:rustc-link-lib=static={}", libzmq);
    println!("cargo:rustc-link-lib=uuid");
    println!("cargo:rustc-link-lib=iphlpapi");
    println!("cargo:rustc-link-lib=Rpcrt4");
}

fn print_linux(out: &std::path::Display<'_>) {
    let root = find_root();
    let header = format!("{}/libcommon/sys/unix/sys.h", root);
    let header = fs::read(header).unwrap();
    fs::write("./sys.h", header).unwrap();
    gen_common_headers(&root);
    println!("{}", out);
    println!("cargo:rustc-link-lib=static=zmq");
    println!("cargo:rustc-link-search=native={}/lib", out);
    println!("cargo:rustc-link-search=native={}/lib64", out);
    println!("cargo:rustc-link-lib=static=linqnetwork");
    println!("cargo:rustc-link-lib=static=linqcommon");
    println!("cargo:rustc-link-lib=static=czmq");
    println!("cargo:rustc-link-lib=static=zmq");
    println!("cargo:rustc-link-lib=uuid");
    println!("cargo:rustc-link-lib=stdc++");
    println!("cargo:rustc-link-lib=m");
    println!("cargo:rustc-link-lib=rt");
}

fn print(out: &std::path::Display<'_>) {
    match env::var("CARGO_CFG_TARGET_OS").as_ref().map(|x| &**x) {
        Ok("linux") => print_linux(out),
        Ok("windows") => print_windows(out),
        _ => panic!("Unknown Host OS!"),
    };
}

fn generator() -> Option<String> {
    // -A "x64"
    match env::var("GENERATOR").as_ref() {
        Ok(r) => Some(r.to_string()),
        _ => None,
    }
}

fn main() {
    let dst = if let Some(generator) = generator() {
        cmake::Config::new(find_root())
            .generator(generator)
            .define("BUILD_LINQD", "OFF")
            .define("WITH_CPP_BINDING", "OFF")
            .build()
    } else {
        cmake::Config::new(find_root())
            .define("BUILD_LINQD", "OFF")
            .define("WITH_CPP_BINDING", "OFF")
            .build()
    };
    let out = dst.display();
    print(&out);

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
