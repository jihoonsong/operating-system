#ifndef VM_SWAPTAB_H
#define VM_SWAPTAB_H

#include <stdbool.h>
#include <stddef.h>

void swaptab_init (void);
size_t swaptab_swap_out (void *kpage);
bool swaptab_swap_in (size_t swap_slot, void *kpage);
void swaptab_free_slot (size_t swap_slot);

#endif /* vm/swaptab.h */
