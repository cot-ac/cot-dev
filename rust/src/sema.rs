//! Re-export CIRSema infrastructure from mlif.
//!
//! The SemaStep trait and CIRSema pass live in mlif so construct crates
//! can depend on them without circular dependencies. This module
//! re-exports them for convenience.

pub use mlif::{CIRSema, SemaState, SemaStep, StepPosition};
