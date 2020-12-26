#ifndef VM_PAGETAB_H
#define VM_PAGETAB_H

#include <stdbool.h>
#include <stdint.h>
#include "filesys/file.h"

struct pagetab *pagetab_create (void);
void pagetab_destroy (struct pagetab *pagetab);
bool pagetab_install_file_page (struct pagetab *pagetab, void *upage,
                                struct file *file, off_t ofs,
                                uint32_t read_bytes, uint32_t zero_bytes,
                                bool writable);
bool pagetab_install_swap_page (struct pagetab *pagetab, void *upage,
                                size_t swap_slot);
bool pagetab_install_zero_page (struct pagetab *pagetab, void *upage,
                                bool writable);
bool pagetab_set_page (uint32_t *pagedir, struct pagetab *pagetab,
                       void *upage, void *kpage, bool writable);
bool pagetab_load_page (uint32_t *pagedir, struct pagetab *pagetab,
                        void *upage);
bool pagetab_is_page_exist (struct pagetab *pagetab, void *upage);
void pagetab_pin_page (struct pagetab *pagetab, void *upage);
void pagetab_unpin_page (struct pagetab *pagetab, void *upage);

#endif /* vm/pagetab.h */
