#include "vm/frametab.h"
#include <debug.h>
#include <list.h>
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

/* A frame table. Frames are inserted in FIFO manner
   for the clock page replacement algorithm. */
static struct list frametab;

/* A frame table entry. */
struct frame
  {
    struct thread *thread;      /* A frame owner thread. */
    void *upage;                /* A user page, which points to KPAGE. */
    void *kpage;                /* A kernel page, which equals to frame. */
    struct list_elem elem;      /* List element. */
  };

/* A frame locks. */
static struct lock get_frame_lock;
static struct lock free_frame_lock;

/* Initialize the frame table and the frame lock. */
void
frametab_init (void)
{
  list_init (&frametab);
  lock_init (&get_frame_lock);
  lock_init (&free_frame_lock);
}

/* Allocate a new frame and return its kernel virtual address.
   If there is no empty slot in the frame table, one frame will be
   selected as a victim and be evicted to allocate a new frame. */
void *
frametab_get_frame (enum palloc_flags flags, void *upage)
{
  if (!lock_held_by_current_thread (&get_frame_lock))
    lock_acquire (&get_frame_lock);

  /* Allocate a new frame. */
  void *kpage = palloc_get_page (PAL_USER | flags);
  if (kpage == NULL)
    {
      // TODO: Swapping have to occur.
      PANIC ("frametab_get_frame: out of frametab");
    }

  /* Construct a new frame table entry. */
  struct frame *frame = malloc (sizeof *frame);
  if (frame == NULL)
    return NULL;

  frame->thread = thread_current ();
  frame->upage = upage;
  frame->kpage = kpage;

  /* Add a new frame table entry to the frame table. */
  list_push_back (&frametab, &frame->elem);

  lock_release (&get_frame_lock);

  return kpage;
}

/* Remove the frame at KPAGE from the frame table and free it. */
void
frametab_free_frame (void *kpage)
{
  if (!lock_held_by_current_thread (&free_frame_lock))
    lock_acquire (&free_frame_lock);

  /* Remove the frame at KPAGE from the frame table. */
  for (struct list_elem *e = list_begin (&frametab);
       e != list_end (&frametab); e = list_next (e))
    {
      struct frame *frame = list_entry (e, struct frame, elem);
      if (frame->kpage == kpage)
        {
          list_remove (&frame->elem);
          break;
        }
    }

  /* Free KPAGE. */
  palloc_free_page (kpage);

  lock_release (&free_frame_lock);
}
