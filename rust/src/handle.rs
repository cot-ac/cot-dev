//! Opaque handle management for the C API.
//!
//! The C API passes opaque pointers (MlirContext, MlirModule, etc.) across
//! the FFI boundary. This module provides safe conversions between Rust
//! types and raw C pointers using Box allocation.
//!
//! Pattern: `Box::into_raw(Box::new(value))` to export,
//! `unsafe { Box::from_raw(ptr) }` to reclaim.

use mlif::Context;

/// Export a Context to a raw pointer for C.
pub fn context_to_raw(ctx: Context) -> *mut Context {
    Box::into_raw(Box::new(ctx))
}

/// Reclaim a Context from a raw C pointer. Caller must ensure validity.
///
/// # Safety
/// The pointer must have been created by `context_to_raw` and not yet freed.
pub unsafe fn context_from_raw(ptr: *mut Context) -> Box<Context> {
    unsafe { Box::from_raw(ptr) }
}

/// Borrow a Context from a raw C pointer.
///
/// # Safety
/// The pointer must be valid and not aliased mutably.
pub unsafe fn context_ref<'a>(ptr: *mut Context) -> &'a mut Context {
    unsafe { &mut *ptr }
}
