#include "vm/pagetab.h"
#include <hash.h>
#include <string.h>
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "vm/frametab.h"

/* How to install pages. */
enum page_install_flag
  {
    PAGE_FILE = 1,           /* Page contents from file system. */
    PAGE_SWAP = 2,           /* Page contents from swap slot. */
    PAGE_ZERO = 3,           /* Zero page contents. */
    PAGE_PRESENT,            /* Page contents present in memory. */
  };

/* A supplemental page table. */
struct pagetab
  {
    struct hash pages;
  };

/* A supplemental page table entry. */
struct page
  {
    void *upage;                  /* A user page, which points to KPAGE. */
    void *kpage;                  /* A kernel page, which equals to frame. */
    bool writable;                /* True if writable, false otherwise. */
    enum page_install_flag flag;  /* Page installation flag. */

    /* If page will be lazily loaded from file system. */
    struct file *file;            /* A file to read. */
    off_t ofs;                    /* An offset. */
    uint32_t read_bytes;          /* The number of bytes to read. */
    uint32_t zero_bytes;          /* The number of bytes to zero. */

    /* If page will be lazily loaded from swap disk. */
    size_t swap_slot;             /* An index of swap slot to swap-in. */

    struct hash_elem elem;        /* Hash element. */
  };

static struct page *pagetab_find_page (struct pagetab *pagetab, void *upage);
static bool pagetab_load_file_page (struct file *file, off_t ofs,
                                    uint32_t read_bytes, uint32_t zero_bytes,
                                    void *frame);
static bool pagetab_load_zero_page (void *frame);
static bool less_func (const struct hash_elem *a,
                       const struct hash_elem *b,
                       void *aux UNUSED);
static unsigned hash_func (const struct hash_elem *e, void *aux UNUSED);

/* Create a new supplemental page table that each entry has an additional
   information along with the mapping between user virtual page and
   kernel virtual page. Return the new supplemental page table. */
struct pagetab *
pagetab_create (void)
{
  struct pagetab *pagetab = malloc (sizeof *pagetab);

  hash_init (&pagetab->pages, hash_func, less_func, NULL);

  return pagetab;
}

/* Install a new page in a supplemental page table that will be lazily
   loaded from file system. */
bool
pagetab_install_file_page (struct pagetab *pagetab, void *upage,
                           struct file *file, off_t ofs,
                           uint32_t read_bytes, uint32_t zero_bytes,
                           bool writable)
{
  /* Construct a new supplemental page table entry. */
  struct page *page = malloc (sizeof *page);

  page->upage = upage;
  page->kpage = NULL;
  page->writable = writable;
  page->flag = PAGE_FILE;

  page->file = file;
  page->ofs = ofs;
  page->read_bytes = read_bytes;
  page->zero_bytes = zero_bytes;

  /* Insert the new entry to a supplemental page table. */
  if (hash_insert (&pagetab->pages, &page->elem) != NULL)
    return false;

  return true;
}

/* Mark a page in a supplemental page table as swapped-out so that it can
   be loaded from swap disk. */
bool pagetab_install_swap_page (struct pagetab *pagetab, void *upage,
                                size_t swap_slot)
{
  /* Find a page that has been swapped-out. */
  struct page *page = pagetab_find_page (pagetab, upage);
  if (page == NULL)
    return false;

  page->kpage = NULL;
  page->flag = PAGE_SWAP;

  /* Indicate where it has been swapped-out to. */
  page->swap_slot = swap_slot;

  return true;
}

/* Install a new page in a supplemental page table that will be lazily
   loaded and be zeroed out. */
bool
pagetab_install_zero_page (struct pagetab *pagetab, void *upage,
                           bool writable)
{
  /* Construct a new supplemental page table entry. */
  struct page *page = malloc (sizeof *page);

  page->upage = upage;
  page->kpage = NULL;
  page->writable = writable;
  page->flag = PAGE_ZERO;

  /* Insert the new entry to a supplemental page table. */
  if (hash_insert (&pagetab->pages, &page->elem) != NULL)
    return false;

  return true;
}

/* Add a mapping in page directory PAGEDIR from user virtual page
   UPAGE to the physical frame identified by kernel virtual address KPAGE
   and add to a supplemental page table entry with additional information.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   If WRITABLE is true, the new page is read/write;
   otherwise it is read-only.
   Returns true if successful, false if memory allocation
   failed. */
bool
pagetab_set_page (uint32_t *pagedir, struct pagetab *pagetab,
                  void *upage, void *kpage, bool writable)
{
  /* If a supplemental page table doesn't have an entry of UPAGE,
     then construct and insert a new entry to a supplemental page table.
     Otherwise, it is lazy loading or swap-in, so just update its mapping. */
  struct page *page = pagetab_find_page (pagetab, upage);
  if (page == NULL)
    {
      /* Construct a new supplemental page table entry. */
      page = malloc (sizeof *page);
      page->upage = upage;

      /* Insert a new entry to a supplemental page table. */
      hash_insert (&pagetab->pages, &page->elem);
    }

  /* Update a supplemental page table entry's mapping. */
  page->kpage = kpage;
  page->writable = writable;
  page->flag = PAGE_PRESENT;

  /* Clear dirty bit. */
  pagedir_set_dirty (pagedir, kpage, false);

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (pagedir, upage) == NULL
          && pagedir_set_page (pagedir, upage, kpage, writable));
}

/* Locate the page that faulted in a supplemental page table,
   obtain a frame to store the page, fetch the data into the frame,
   and update a supplemental page table entry point to the physical page. */
bool
pagetab_load_page (uint32_t *pagedir, struct pagetab *pagetab,
                   void *upage)
{
  /* Locate the page that faulted in a supplemental page table. */
  struct page *fault_page = pagetab_find_page (pagetab, upage);
  if (fault_page == NULL)
    return false;

  /* Obtain a frame to store the page. */
  void *frame = frametab_get_frame (PAL_USER, upage);
  if (frame == NULL)
    return false;

  /* Fetch the data into the frame. */
  switch (fault_page->flag)
    {
      case PAGE_FILE:
        if (!pagetab_load_file_page (fault_page->file,
                                     fault_page->ofs,
                                     fault_page->read_bytes,
                                     fault_page->zero_bytes,
                                     frame))
          {
            frametab_free_frame (frame);
            return false;
          }
        break;
      case PAGE_SWAP:
        // TODO: Swap-in.
        break;
      case PAGE_ZERO:
        if (!pagetab_load_zero_page (frame))
          {
            frametab_free_frame (frame);
            return false;
          }
        break;
      default:
        /* Do nothing. */
        break;
    }

  /* Update a supplemental page table entry point to the physical page. */
  if (!pagetab_set_page (pagedir, pagetab, upage, frame, fault_page->writable))
    {
      frametab_free_frame (frame);
      return false;
    }

  return true;
}

/* Find and return a supplement page table entry that has user virtual page
   equals to UPAGE. Return NULL if cannot find such entry. */
static struct page *
pagetab_find_page (struct pagetab *pagetab, void *upage)
{
  struct page page = (struct page) {.upage = upage};

  struct hash_elem *elem = hash_find (&pagetab->pages, &page.elem);
  if (elem == NULL)
    return NULL;

  return hash_entry (elem, struct page, elem);
}

/* Read READ_BYTES from FILE into the FRAME. This function is called when
   fetching the data of file page into the FRAME. */
static bool
pagetab_load_file_page (struct file *file, off_t ofs,
                        uint32_t read_bytes, uint32_t zero_bytes,
                        void *frame)
{
  /* The sum of READ_BYTES and ZERO_BYTES should equals to PGSIZE. */
  if (read_bytes + zero_bytes != PGSIZE)
    return false;

  /* Place cursor at the file. */
  file_seek (file, ofs);

  /* Read the data from the file to frame. */
  if ((uint32_t) file_read (file, frame, read_bytes) != read_bytes)
    return false;

  /* Zero the remaining memory in frame. */
  memset (frame + read_bytes, 0, zero_bytes);

  return true;
}

/* Zero the FRAME. This function is called when fetching the data of
   zero page into the FRAME. */
static bool
pagetab_load_zero_page (void *frame)
{
  memset (frame, 0, PGSIZE);

  return true;
}

/* Compares the value of two list elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
static bool
less_func (const struct hash_elem *a,
           const struct hash_elem *b,
           void *aux UNUSED)
{
  struct page *page_a = hash_entry (a, struct page, elem);
  struct page *page_b = hash_entry (b, struct page, elem);

  return page_a->upage < page_b->upage;
}

/* Computes and returns the hash value for hash element E, given
   auxiliary data AUX. */
static unsigned
hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  struct page *page = hash_entry (e, struct page, elem);

  return hash_int ((int) page->upage);
}
