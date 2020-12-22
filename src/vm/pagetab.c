#include "vm/pagetab.h"
#include <hash.h>
#include "threads/malloc.h"
#include "userprog/pagedir.h"

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

    struct hash_elem elem;        /* Hash element. */
  };

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
  /* Construct a new supplemental page table entry. */
  struct page *page = malloc (sizeof *page);

  page->upage = upage;
  page->kpage = kpage;
  page->writable = writable;
  page->flag = PAGE_PRESENT;

  /* Insert the new entry to a supplemental page table. */
  if (hash_insert (&pagetab->pages, &page->elem) != NULL)
    return false;

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (pagedir, upage) == NULL
          && pagedir_set_page (pagedir, upage, kpage, writable));
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
