#include "vm/frametab.h"
#include <debug.h>
#include <list.h>
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "vm/pagetab.h"
#include "vm/swaptab.h"

/* A frame table. Frames are inserted in FIFO manner
   for the clock page replacement algorithm. */
static struct list frametab;

/* A frame table entry. */
struct frame
  {
    struct thread *thread;      /* A frame owner thread. */
    void *upage;                /* A user page, which points to KPAGE. */
    void *kpage;                /* A kernel page, which equals to frame. */
    bool pinned;                /* True if pinned, false otherwise. */
    struct list_elem elem;      /* List element. */
  };

/* A frame locks. */
static struct lock get_frame_lock;
static struct lock free_frame_lock;
static struct lock pin_frame_lock;

static struct frame *frametab_select_victim (void);
static struct frame *frametab_find_frame (void *kpage);

/* Initialize the frame table and the frame lock. */
void
frametab_init (void)
{
  list_init (&frametab);
  lock_init (&get_frame_lock);
  lock_init (&free_frame_lock);
  lock_init (&pin_frame_lock);
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
  while (kpage == NULL)
    {
      /* Select a frame to swap-out. */
      struct frame *victim = frametab_select_victim ();

      /* Swap-out the selected frame. */
      size_t swap_slot = swaptab_swap_out (victim->kpage);

      /* Mark that it has been swapped-out. */
      pagetab_install_swap_page (victim->thread->pagetab, victim->upage,
                                 swap_slot);

      /* Clear its mapping. */
      pagedir_clear_page (victim->thread->pagedir, victim->upage);

      /* Release its memory. */
      list_remove (&victim->elem);
      palloc_free_page (victim->kpage);
      free (victim);

      /* Try to allocate a new frame. */
      kpage = palloc_get_page (PAL_USER | flags);
    }

  /* Construct a new frame table entry. */
  struct frame *frame = malloc (sizeof *frame);
  if (frame == NULL)
    return NULL;

  frame->thread = thread_current ();
  frame->upage = upage;
  frame->kpage = kpage;
  frame->pinned = false;

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
          free (frame);
          break;
        }
    }

  /* Free KPAGE. */
  palloc_free_page (kpage);

  lock_release (&frame_lock);
}

/* Pin frame to prevent swap-out. */
void
frametab_pin_frame (void *kpage)
{
  lock_acquire (&frame_lock);

  struct frame *frame = frametab_find_frame (kpage);
  if (frame == NULL)
    return;

  frame->pinned = true;

  lock_release (&frame_lock);
}

/* Unpin frame to allow swap-out. */
void
frametab_unpin_frame (void *kpage)
{
  lock_acquire (&frame_lock);

  struct frame *frame = frametab_find_frame (kpage);
  if (frame == NULL)
    return;

  frame->pinned = false;

  lock_release (&frame_lock);
}

/* Select a frame to evict using the clock algorithm. */
static struct frame *
frametab_select_victim (void)
{
  /* Clock hand points to the next element to the victim. */
  static struct list_elem *clock_hand = NULL;
  if (clock_hand == NULL)
    clock_hand = list_begin (&frametab);

  /* Select a frame to evict. */
  struct frame *victim = NULL;
  for (size_t i = 0; i <= (list_size (&frametab) << 1); ++i)
  {
    /* Nominate the frame pointed by clock hand as a candidate for a victim */
    victim = list_entry (clock_hand, struct frame, elem);

    /* Move clock hand. */
    clock_hand = list_next (clock_hand);
    if (clock_hand == list_end (&frametab))
      clock_hand = list_begin (&frametab);

    /* Do not swap-out pinned frame. */
    if (victim->pinned)
      continue;

    /* If the candidate has an accessed bit 0, then select it as a victim. */
    if (!pagedir_is_accessed (victim->thread->pagedir, victim->upage))
      break;

    /* If not, clear its accessed bit. */
    pagedir_set_accessed (victim->thread->pagedir, victim->upage, false);
  }

  /* Update clock hand. */
  clock_hand = list_next (&victim->elem);

  return victim;
}

/* Find and return a frame that has kernel virtual page
   equals to KPAGE. Return NULL if cannot find such frame. */
static struct frame *
frametab_find_frame (void *kpage)
{
  for (struct list_elem *e = list_begin (&frametab);
       e != list_end (&frametab); e = list_next (e))
    {
      struct frame *frame = list_entry (e, struct frame, elem);
      if (frame->kpage == kpage)
        return frame;
    }

  return NULL;
}
