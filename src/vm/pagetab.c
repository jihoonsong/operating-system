#include "vm/pagetab.h"
#include <hash.h>
#include "threads/malloc.h"

/* A supplemental page table. */
struct pagetab
  {
    struct hash pages;
  };

/* A supplemental page table entry. */
struct page
  {
    void *upage;                /* A user page, which points to KPAGE. */
    void *kpage;                /* A kernel page, which equals to frame. */
    struct hash_elem elem;      /* Hash element. */
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
