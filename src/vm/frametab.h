#ifndef VM_FRAMETAB_H
#define VM_FRAMETAB_H

#include "threads/palloc.h"

void frametab_init (void);
void *frametab_get_frame (enum palloc_flags flags, void *upage);
void frametab_free_frame (void *kpage);

void frametab_pin_frame (void *kpage);
void frametab_unpin_frame (void *kpage);

#endif /* vm/frametab.h */
