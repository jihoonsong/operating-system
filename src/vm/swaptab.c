#include "vm/swaptab.h"
#include <bitmap.h>
#include "devices/block.h"
#include "threads/vaddr.h"

#define BLOCKS_PER_SLOT (PGSIZE / BLOCK_SECTOR_SIZE)

/* A swap table. */
static struct swap swaptab;

/* A swap table entry. */
struct swap
  {
    struct block *swap_blocks;
    struct bitmap *swap_slots;
  };

/* Initialize the swap table. */
void
swaptab_init (void)
{
  /* Allocate swap disk. */
  swaptab.swap_blocks = block_get_role (BLOCK_SWAP);

  /* Prepare swap slots. Each swap slot has a contiguous blocks
     where their sum equals to PGSIZE.*/
  size_t swaptab_size = block_size (swaptab.swap_blocks) / BLOCKS_PER_SLOT;
  swaptab.swap_slots = bitmap_create (swaptab_size);

  /* Initialize swap slots to true. */
  bitmap_set_all (swaptab.swap_slots, true);
}

/* Find an empty swap slot and swap-out KPAGE to the swap slot. */
size_t
swaptab_swap_out (void *kpage)
{
  /* Find an empty swap slot. */
  size_t swap_slot = bitmap_scan (swaptab.swap_slots, 0, BLOCKS_PER_SLOT, true);

  /* Swap-out KPAGE to the swap slot. */
  for (size_t i = 0; i < BLOCKS_PER_SLOT; ++i)
    block_write (swaptab.swap_blocks,
                 swap_slot * BLOCKS_PER_SLOT + i,
                 kpage + BLOCK_SECTOR_SIZE * i);

  /* Mark the swap slot as used. */
  bitmap_set (swaptab.swap_slots, swap_slot, false);

  return swap_slot;
}

/* Swap-in SWAP_SLOT to KPAGE. */
bool
swaptab_swap_in (size_t swap_slot, void *kpage)
{
  if (swap_slot >= bitmap_size (swaptab.swap_slots) ||
      bitmap_test(swaptab.swap_slots, swap_slot))
    return false;

  /* Swap-in SWAP_SLOT to KPAGE. */
  for (size_t i = 0; i < BLOCKS_PER_SLOT; ++i)
    block_read (swaptab.swap_blocks,
                swap_slot * BLOCKS_PER_SLOT + i,
                kpage + BLOCK_SECTOR_SIZE * i);

  /* Mark the SWAP_SLOT as empty. */
  bitmap_set (swaptab.swap_slots, swap_slot, true);

  return true;
}
