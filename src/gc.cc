#include "gc.h"
#include "heap.h"
#include "heap-inl.h"

#include <sys/types.h> // off_t
#include <stdlib.h> // NULL
#include <assert.h> // assert

namespace candor {

void GC::GCValue::Relocate(char* address) {
  if (slot_ != NULL) {
    *slot_ = address;
    value()->SetGCMark(address);
  }
}

void GC::CollectGarbage(char* stack_top) {
  assert(grey_items()->length() == 0);

  // Temporary space which will contain copies of all visited objects
  Space space(heap(), heap()->new_space()->page_size());

  // Reset GC flag
  heap()->needs_gc(0);

  // Go through the stack
  char* top = stack_top;
  for (; top != NULL; top += sizeof(void*)) {
    char** slot = reinterpret_cast<char**>(top);

    // Once found enter frame signature
    // skip stack entities until last exit frame position (or NULL)
    while (top != NULL && *reinterpret_cast<uint32_t*>(slot) == 0xFEEEDBEE) {
      top = *reinterpret_cast<char**>(top + sizeof(void*));
      slot = reinterpret_cast<char**>(top);
    }
    if (top == NULL) break;

    // Skip rbp as well
    if ((*reinterpret_cast<uint32_t*>(slot + 1) & 0x8000000) == 0 &&
        HValue::Cast(*(slot + 1))->tag() == Heap::kTagCode) {
      top += sizeof(void*);
      continue;
    }

    char* value = *slot;

    // Skip NULL pointers, non-pointer values and rbp pushes
    if (value == NULL || HValue::IsUnboxed(value)) continue;

    // Ignore return addresses
    HValue* hvalue = HValue::Cast(value);
    if (hvalue == NULL || hvalue->tag() == Heap::kTagCode) continue;

    grey_items()->Push(new GCValue(hvalue, slot));
  }

  while (grey_items()->length() != 0) {
    GCValue* value = grey_items()->Shift();

    // Skip unboxed address
    if (HValue::IsUnboxed(value->value()->addr())) continue;

    if (!value->value()->IsGCMarked()) {
      HValue* hvalue = value->value()->CopyTo(&space);
      value->Relocate(hvalue->addr());
      GC::VisitValue(hvalue);
    } else {
      value->Relocate(value->value()->GetGCMark());
    }
  }

  heap()->new_space()->Swap(&space);
}


void GC::VisitValue(HValue* value) {
  switch (value->tag()) {
   case Heap::kTagContext:
    return VisitContext(value->As<HContext>());
   case Heap::kTagFunction:
    return VisitFunction(value->As<HFunction>());
   case Heap::kTagObject:
    return VisitObject(value->As<HObject>());
   case Heap::kTagMap:
    return VisitMap(value->As<HMap>());

   // String and numbers ain't referencing anyone
   case Heap::kTagString:
   case Heap::kTagNumber:
   case Heap::kTagBoolean:
    return;
   default:
    UNEXPECTED
  }
}


void GC::VisitContext(HContext* context) {
  if (context->has_parent()) {
    grey_items()->Push(
        new GCValue(HValue::Cast(context->parent()), context->parent_slot()));
  }

  for (uint32_t i = 0; i < context->slots(); i++) {
    if (!context->HasSlot(i)) continue;

    HValue* value = context->GetSlot(i);
    grey_items()->Push(new GCValue(value, context->GetSlotAddress(i)));
  }
}


void GC::VisitFunction(HFunction* fn) {
  grey_items()->Push(new GCValue(HValue::Cast(fn->parent()), fn->parent_slot()));
}


void GC::VisitObject(HObject* obj) {
  grey_items()->Push(new GCValue(HValue::Cast(obj->map()), obj->map_slot()));
}


void GC::VisitMap(HMap* map) {
  for (uint32_t i = 0; i < map->size(); i++) {
    if (map->IsEmptySlot(i)) continue;
    grey_items()->Push(new GCValue(map->GetSlot(i),
                                   map->GetSlotAddress(i)));
    grey_items()->Push(new GCValue(map->GetSlot(i + map->size()),
                                   map->GetSlotAddress(i + map->size())));
  }
}

} // namespace candor
