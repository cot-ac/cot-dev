//! CIR C API — 145 `#[no_mangle] extern "C"` functions.
//!
//! Implements the same signatures as `cot-dev/include/cot-c/CIRCApi.h`.
//! Each function creates MLIF operations, types, or inspects IR — backed
//! by the Rust construct crates instead of C++/MLIR.
//!
//! ## Implementation phases:
//!
//! - Phase 4: Core ops (constants, arithmetic, memory, flow, functions)
//! - Phase 5: Aggregate types (structs, arrays, slices, optionals, errors, enums, unions)
//! - Phase 6: Advanced (generics, traits, vwt, test)

// TODO: Phase 4 — implement 145 extern "C" functions matching CIRCApi.h
