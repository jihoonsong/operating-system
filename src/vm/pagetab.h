#ifndef VM_PAGETAB_H
#define VM_PAGETAB_H

#include <stdbool.h>
#include <stdint.h>

struct pagetab *pagetab_create (void);
bool pagetab_set_page (uint32_t *pagedir, struct pagetab *pagetab,
                       void *upage, void *kpage, bool writable);

#endif /* vm/pagetab.h */
