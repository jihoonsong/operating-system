#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#ifdef USERPROG
#include "userprog/process.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running.
   Sorted by priority in monotone decreasing order. */
static struct list ready_list;

/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

#ifndef USERPROG
bool thread_prior_aging;
#endif

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;

/* A fraction for fixed-point number in signed 17.14 format. */
static int fraction;

/* The system load average, an estimation of the average number of threads
   ready to run over the past minute except for the idle thread. */
static int load_avg;

/* The number of threads that are running or ready except for IDLE_THREAD. */
static int ready_threads;

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void thread_schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);

#ifndef USERPROG
static void thread_aging (void);
#endif
static bool ready_list_compare (const struct list_elem *a,
                                const struct list_elem *b,
                                void *aux);

/* Calculate priority of THREAD determined by the formula of BSD scheduler. */
static int calculate_priority (const struct thread *thread);

/* Helper functions for fixed-point real arithmetic. */
#define INT
#define REAL
static int int_to_real (int INT n);
static int real_to_int (int REAL x);
static int add_real_and_int (int REAL x, int INT n);
static int add_real_and_real (int REAL x, int REAL y);
static int sub_int_from_real (int INT n, int REAL x);
static int sub_real_from_real (int REAL y, int REAL x);
static int mul_real_by_real (int REAL x, int REAL y);
static int mul_real_by_int (int REAL x, int INT n);
static int div_real_by_real (int REAL x, int REAL y);
static int div_real_by_int (int REAL x, int INT n);

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */
void
thread_init (void)
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&all_list);

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
  initial_thread->nice = 0;
  initial_thread->recent_cpu = 0;

  /* Set up a fraction for fixed-point number in signed 17.14 format. */
  fraction = 1 << 14;

  /* Set up data for BSD scheduler. */
  load_avg = 0;
  ready_threads = 0;
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void)
{
  /* Create the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create ("idle", PRI_MIN, idle, &idle_started);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void)
{
  struct thread *t = thread_current ();

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();

#ifndef USERPROG
  if (thread_prior_aging)
    thread_aging ();
#endif
}

/* Prints thread statistics. */
void
thread_print_stats (void)
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux)
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
#ifdef USERPROG
  struct process *pcb;
#endif
  tid_t tid;

  ASSERT (function != NULL);

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();
  t->nice = thread_current ()->nice;
  t->recent_cpu = thread_current ()->recent_cpu;

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;
  sf->ebp = 0;

#ifdef USERPROG
  /* Create a process control block. */
  pcb = palloc_get_page (PAL_ZERO);
  if (pcb == NULL)
    return TID_ERROR;

  pcb->pid = (pid_t) tid;
  pcb->alive = true;
  pcb->being_waited = false;
  pcb->start_success = false;
  pcb->exit_status = -1;
  sema_init (&pcb->start, 0);
  sema_init (&pcb->wait, 0);

  /* Make a new thread points to its corresponding process control block. */
  t->pcb = pcb;

  /* Make current thread points to a process control block of a new thread. */
  list_push_back (&thread_current ()->children, &pcb->elem);
#endif

  /* Add to run queue. */
  thread_unblock (t);

#ifdef USERPROG
  /* Wait until a new thread starts its execution. */
  sema_down (&pcb->start);

  /* Check if its execution started successfully. */
  if (!pcb->start_success)
    return TID_ERROR;
#endif

  /* Yield CPU if the priority of current thread is not the maximum priority. */
  if (thread_current ()->priority < t->priority)
    thread_yield ();

  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void)
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  struct thread *cur = thread_current ();
  cur->status = THREAD_BLOCKED;
  if (cur != idle_thread)
    --ready_threads;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t)
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  list_insert_ordered (&ready_list, &t->elem, ready_list_compare, NULL);
  t->status = THREAD_READY;

  /* Update READY_THREADS. */
  if (t != idle_thread)
    ++ready_threads;

  intr_set_level (old_level);
}

/* Returns the name of the running thread. */
const char *
thread_name (void)
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void)
{
  struct thread *t = running_thread ();

  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void)
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void)
{
  ASSERT (!intr_context ());

#ifdef USERPROG
  process_exit ();
#endif

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  intr_disable ();
  struct thread *cur = thread_current ();
  list_remove (&cur->allelem);
  cur->status = THREAD_DYING;
  if (cur != idle_thread)
    --ready_threads;
  schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void)
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;

  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread)
    list_insert_ordered (&ready_list, &cur->elem, ready_list_compare, NULL);
  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void
thread_foreach (thread_action_func *func, void *aux)
{
  struct list_elem *e;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
    {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority)
{
  struct thread *cur = thread_current ();

  cur->base_priority = new_priority;

  cur->priority = thread_find_max_priority (cur);

  /* Preempts the current running thread if it has a lower priority than
     the head of ready_list. */
  if (cur != idle_thread && !list_empty (&ready_list))
    {
      struct thread *ready_front = list_entry (list_front (&ready_list),
                                               struct thread, elem);
      if (cur->priority < ready_front->priority)
        thread_yield ();
    }
}

/* Returns the maximum priority of THREAD among its base priority and
   donated priorities. */
int
thread_find_max_priority (struct thread *thread)
{
  int max_priority = thread->base_priority;

  for (struct list_elem *e = list_begin (&thread->donated_priorities);
       e != list_end (&thread->donated_priorities);
       e = list_next (e))
    {
      struct donated_priority *donation = \
        list_entry (e, struct donated_priority, elem);

      if (max_priority < donation->priority)
        max_priority = donation->priority;
    }

  return max_priority;
}

/* Updates priorities of all threads. */
void
thread_update_priority (void)
{
  if (!thread_mlfqs)
    return;

  static int ticks = 0;

  /* Perform update per TIME_SLICE. */
  if (++ticks != TIME_SLICE)
    return;

  ticks = 0;

  int max_priority = PRI_MIN;
  for (struct list_elem *e = list_begin (&all_list);
       e != list_end (&all_list); e = list_next (e))
    {
      struct thread *thread = list_entry (e, struct thread, allelem);

      thread->priority = calculate_priority (thread);

      if (max_priority < thread->priority)
        max_priority = thread->priority;
    }

  /* Yield CPU if the priority of current thread is not the maximum priority.

     Note that intr_yield_on_return () must be used instead of thread_yield ()
     becuase this function is called within timer_interrupt (). */
  if (thread_current ()->priority < max_priority)
    intr_yield_on_return();
}

/* Returns the current thread's priority. */
int
thread_get_priority (void)
{
  return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice)
{
  ASSERT (thread_mlfqs);
  ASSERT (nice >= NICE_MIN && nice <= NICE_MAX);

  struct thread *cur = thread_current ();
  if (cur == idle_thread)
    return;

  cur->nice = nice;
  cur->priority = calculate_priority (cur);

  /* Yield CPU if the priority of current thread is not the maximum priority. */
  if (!list_empty (&ready_list))
    {
      struct thread *ready_front = list_entry (list_front (&ready_list),
                                               struct thread, elem);
      if (cur->priority < ready_front->priority)
        thread_yield ();
    }
}

/* Returns the current thread's nice value. */
int
thread_get_nice (void)
{
  ASSERT (thread_mlfqs);

  return thread_current ()->nice;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void)
{
  ASSERT (thread_mlfqs);
  ASSERT (load_avg >= 0);

  return real_to_int (load_avg * 100);
}

/* Update the system load average. */
void
thread_update_load_avg (void)
{
  if (!thread_mlfqs)
    return;

  /* Update the system load average. Please be cautious on
     fixed-point arithmetic operations. */
  int load_avg_coef = div_real_by_int (int_to_real (59), 60);
  int ready_threads_coef = div_real_by_int (int_to_real (1), 60);
  int weighted_load_avg = mul_real_by_real (load_avg_coef, load_avg);
  int weighted_ready_threads = mul_real_by_int (ready_threads_coef,
                                                ready_threads);
  load_avg = add_real_and_real (weighted_load_avg, weighted_ready_threads);
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void)
{
  ASSERT (thread_mlfqs);

  return real_to_int (thread_current ()->recent_cpu * 100);
}

/* Increment RECENT_CPU of the current running thread by 1. */
void
thread_increment_recent_cpu (void)
{
  if (!thread_mlfqs)
    return;

  struct thread *cur = thread_current ();
  if (cur != idle_thread)
    cur->recent_cpu = add_real_and_int (cur->recent_cpu, 1);
}

/* Update RECENT_CPU of all threads except for IDLE_THREAD. */
void
thread_update_recent_cpu (void)
{
  if (!thread_mlfqs)
    return;

  int load_avg_twice = mul_real_by_int (load_avg, 2);
  int recent_cpu_coef = div_real_by_real (load_avg_twice,
                                          add_real_and_int (load_avg_twice, 1));

  for (struct list_elem *e = list_begin (&all_list);
       e != list_end (&all_list); e = list_next (e))
    {
      struct thread *thread = list_entry (e, struct thread, allelem);
      if (thread == idle_thread)
        continue;

      int recent_cpu = thread->recent_cpu;
      int weighted_recent_cpu = mul_real_by_real (recent_cpu_coef, recent_cpu);
      thread->recent_cpu = add_real_and_int (weighted_recent_cpu, thread->nice);
    }
}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED)
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
#ifdef USERPROG
  idle_thread->pcb->start_success = true;
  sema_up (&idle_thread->pcb->start);
#endif
  sema_up (idle_started);

  for (;;)
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux)
{
  ASSERT (function != NULL);

  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
running_thread (void)
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Because `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  enum intr_level old_level;

  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;
  t->priority = priority;
  t->base_priority = priority;
  list_init (&t->donated_priorities);
  t->waiting_on_lock = NULL;
  t->magic = THREAD_MAGIC;

#ifdef USERPROG
  /* Initialize a list of process control blocks of children and files
     and a pointer to the ELF executable.

     Initialization must be done here because even main thread can have
     children and open files. Only main thread doesn't have ELF executable. */
  list_init (&t->children);
  list_init (&t->files);
  t->elf_executable = NULL;
#endif

  old_level = intr_disable ();
  list_push_back (&all_list, &t->allelem);
  intr_set_level (old_level);
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size)
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void)
{
  if (list_empty (&ready_list))
    return idle_thread;
  else
    {
      list_sort (&ready_list, ready_list_compare, NULL);
      return list_entry (list_pop_front (&ready_list), struct thread, elem);
    }
}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();

  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread)
    {
      ASSERT (prev != cur);
      palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.

   It's not safe to call printf() until thread_schedule_tail()
   has completed. */
static void
schedule (void)
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void)
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);

#ifndef USERPROG
static void
thread_aging (void)
{
  for (struct list_elem *e = list_begin (&ready_list);
       e != list_end (&ready_list); e = list_next (e))
    {
      struct thread *thread = list_entry (e, struct thread, elem);
      thread->base_priority += 1;
      thread->priority = thread_find_max_priority (thread);
    }
}
#endif

/* Compares the value of two list elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
static bool
ready_list_compare (const struct list_elem *a,
                    const struct list_elem *b,
                    void *aux UNUSED)
{
  struct thread *thread_a = list_entry (a, struct thread, elem);
  struct thread *thread_b = list_entry (b, struct thread, elem);

  return thread_a->priority > thread_b->priority;
}

/* Calculate priority of THREAD determined by the formula of BSD scheduler. */
static int
calculate_priority (const struct thread *thread)
{
  ASSERT (thread != NULL);

  /* Calculate priority. */
  int priority = int_to_real (PRI_MAX);
  priority = sub_real_from_real (div_real_by_int (thread->recent_cpu, 4),
                                 priority);
  priority = sub_int_from_real (thread->nice * 2, priority);
  priority = real_to_int (priority);

  /* Calibrate priority. */
  priority = PRI_MIN > priority ? PRI_MIN : priority;
  priority = PRI_MAX < priority ? PRI_MAX : priority;

  return priority;
}

static int
int_to_real (int INT n)
{
  return n * fraction;
}

static int
real_to_int (int REAL x)
{
  /* Round to the nearest integer. */
  x += x > 0 ? fraction / 2 : -fraction / 2;
  return x / fraction;
}

static int
add_real_and_int (int REAL x, int INT n)
{
  return x + n * fraction;
}

static int
add_real_and_real (int REAL x, int REAL y)
{
  return x + y;
}

static int
sub_int_from_real (int INT n, int REAL x)
{
  return x - n * fraction;
}

static int
sub_real_from_real (int REAL y, int REAL x)
{
  return x - y;
}

static int
mul_real_by_real (int REAL x, int REAL y)
{
  return ((int64_t) x) * y / fraction;
}

static int
mul_real_by_int (int REAL x, int INT n)
{
  return x * n;
}

static int
div_real_by_real (int REAL x, int REAL y)
{
  return ((int64_t) x) * fraction / y;
}

static int
div_real_by_int (int REAL x, int INT n)
{
  return x / n;
}
