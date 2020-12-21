#include "vm/frametab.h"
#include <list.h>
#include "threads/synch.h"

/* A frame table. Frames are inserted in FIFO manner
   for the clock page replacement algorithm. */
static struct list frames;

/* A frame lock. */
static struct lock frame_lock;

/* Initialize the frame table and the frame lock. */
void
frametab_init (void)
{
  list_init (&frames);
  lock_init (&frame_lock);
}
