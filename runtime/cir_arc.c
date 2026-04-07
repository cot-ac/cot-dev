//===- cir_arc.c - ARC runtime implementation -----------------*- C -*-===//
//
// Reference: Swift runtime RefCount.cpp, HeapObject.cpp
// Phase 1: atomic variants delegate to nonatomic.
//
//===----------------------------------------------------------------------===//
#include "cir_arc.h"

#include <stdlib.h>
#include <string.h>

//===----------------------------------------------------------------------===//
// Allocation
//===----------------------------------------------------------------------===//

CirHeapObject *cir_allocObject(void *metadata, size_t size,
                               size_t alignMask) {
  size_t alignment = alignMask + 1;
  void *mem = aligned_alloc(alignment, (size + alignMask) & ~alignMask);
  if (!mem)
    return NULL;

  CirHeapObject *obj = (CirHeapObject *)mem;
  obj->metadata = metadata;
  // Initial refcount: strong=1 (bits 0-29 = 1), unowned=1 (bit 32 = 1)
  obj->refCounts = 1ULL | (1ULL << CIR_UNOWNED_SHIFT);
  return obj;
}

void cir_deallocObject(CirHeapObject *object, size_t size,
                       size_t alignMask) {
  (void)size;
  (void)alignMask;
  free(object);
}

//===----------------------------------------------------------------------===//
// Strong reference counting
//===----------------------------------------------------------------------===//

void cir_nonatomic_retain(CirHeapObject *object) {
  if (!object)
    return;
  // Skip immortal objects
  if (object->refCounts == CIR_IMMORTAL_REFCOUNTS)
    return;
  object->refCounts += 1;
}

void cir_retain(CirHeapObject *object) {
  // Phase 1: atomic delegates to nonatomic
  cir_nonatomic_retain(object);
}

void cir_nonatomic_release(CirHeapObject *object) {
  if (!object)
    return;
  // Skip immortal objects
  if (object->refCounts == CIR_IMMORTAL_REFCOUNTS)
    return;

  uint32_t strong =
      (uint32_t)(object->refCounts & CIR_STRONG_MASK);
  if (strong <= 1) {
    // Mark as deiniting
    object->refCounts |= CIR_IS_DEINITING_BIT;
    // Free the object
    free(object);
    return;
  }
  object->refCounts -= 1;
}

void cir_release(CirHeapObject *object) {
  // Phase 1: atomic delegates to nonatomic
  cir_nonatomic_release(object);
}

//===----------------------------------------------------------------------===//
// Unowned reference counting
//===----------------------------------------------------------------------===//

void cir_unownedRetain(CirHeapObject *object) {
  if (!object)
    return;
  if (object->refCounts == CIR_IMMORTAL_REFCOUNTS)
    return;
  object->refCounts += (1ULL << CIR_UNOWNED_SHIFT);
}

void cir_unownedRelease(CirHeapObject *object) {
  if (!object)
    return;
  if (object->refCounts == CIR_IMMORTAL_REFCOUNTS)
    return;
  object->refCounts -= (1ULL << CIR_UNOWNED_SHIFT);
}

//===----------------------------------------------------------------------===//
// Weak references (stubbed for Phase 1)
//===----------------------------------------------------------------------===//

void cir_weakInit(CirHeapObject **weakRef, CirHeapObject *object) {
  *weakRef = object;
}

CirHeapObject *cir_weakLoadStrong(CirHeapObject **weakRef) {
  CirHeapObject *obj = *weakRef;
  if (obj)
    cir_retain(obj);
  return obj;
}

void cir_weakDestroy(CirHeapObject **weakRef) {
  *weakRef = NULL;
}
