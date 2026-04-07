//===- cir_arc.h - ARC runtime for heap objects ----------------*- C -*-===//
//
// Reference: Swift runtime HeapObject.h
//
//===----------------------------------------------------------------------===//
#ifndef CIR_ARC_H
#define CIR_ARC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Heap object header. Every ARC-managed object starts with this.
/// Reference: Swift HeapObject — metadata pointer + refcount word.
typedef struct {
  void *metadata;
  uint64_t refCounts;
} CirHeapObject;

/// Bit layout for refCounts word.
/// Reference: Swift InlineRefCounts.h
#define CIR_STRONG_MASK       0x000000003FFFFFFFULL  // bits 0-29
#define CIR_IS_DEINITING_BIT  (1ULL << 30)
#define CIR_USE_SLOW_RC_BIT   (1ULL << 31)
#define CIR_UNOWNED_SHIFT     32
#define CIR_UNOWNED_MASK      0x7FFFFFFF00000000ULL  // bits 32-62
#define CIR_PURE_SWIFT_BIT    (1ULL << 63)
#define CIR_IMMORTAL_REFCOUNTS 0x80000004FFFFFFFFULL

/// Allocate a new heap object with the given size and alignment.
/// Initializes refCounts to 0x3 (strong=3 to account for initial +1).
CirHeapObject *cir_allocObject(void *metadata, size_t size,
                               size_t alignMask);

/// Deallocate a heap object.
void cir_deallocObject(CirHeapObject *object, size_t size,
                       size_t alignMask);

/// Increment strong reference count (atomic).
void cir_retain(CirHeapObject *object);

/// Increment strong reference count (non-atomic).
void cir_nonatomic_retain(CirHeapObject *object);

/// Decrement strong reference count (atomic).
/// Calls deinit + free when count reaches zero.
void cir_release(CirHeapObject *object);

/// Decrement strong reference count (non-atomic).
void cir_nonatomic_release(CirHeapObject *object);

/// Unowned reference operations.
void cir_unownedRetain(CirHeapObject *object);
void cir_unownedRelease(CirHeapObject *object);

/// Weak reference operations (stubbed for Phase 1).
void cir_weakInit(CirHeapObject **weakRef, CirHeapObject *object);
CirHeapObject *cir_weakLoadStrong(CirHeapObject **weakRef);
void cir_weakDestroy(CirHeapObject **weakRef);

#ifdef __cplusplus
}
#endif

#endif // CIR_ARC_H
