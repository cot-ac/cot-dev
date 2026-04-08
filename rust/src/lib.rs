//! # cot-dev
//!
//! COT framework — C API + pipeline backed by Rust/MLIF/Cranelift.
//!
//! Produces `libcot_dev.a` — a static library implementing the same
//! 159 C ABI functions as `cot-dev/cpp/c-api/` (C++/MLIR/LLVM).
//! Frontends link one backend or the other; the C API headers are shared.
//!
//! ## Modules
//!
//! - `cir_api` — 145 `#[no_mangle] extern "C"` functions for IR building
//! - `cot_api` — 14 `#[no_mangle] extern "C"` functions for pipeline/codegen
//! - `handle` — Opaque pointer management (Box → raw ptr → Box)

pub mod cir_api;
pub mod cot_api;
pub mod handle;

// Reference all construct crates so they're linked into the static library.
extern crate cot_arith;
extern crate cot_memory;
extern crate cot_flow;
extern crate cot_structs;
extern crate cot_arrays;
extern crate cot_slices;
extern crate cot_optionals;
extern crate cot_errors;
extern crate cot_enums;
extern crate cot_unions;
extern crate cot_generics;
extern crate cot_traits;
extern crate cot_vwt;
extern crate cot_test;
