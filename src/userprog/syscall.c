#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

static int get_user (const uint8_t *uaddr);
static void indirect_user (const void *uptr, void *uindirect);
static bool put_user (uint8_t *udst, uint8_t byte);
static void *validate_ptr (void *ptr);
static int allocate_fd (void);

static void syscall_handler (struct intr_frame *);
static void halt (void);
static void exit (int status);
static tid_t exec (const char *task);
static int wait (tid_t tid);
static bool create (const char *filename, unsigned initial_size);
static bool remove (const char *filename);
static int open (const char *filename);
static int filesize (int fd);
static int read (int fd, void *buffer, unsigned int size);
static int write (int fd, const void *buffer, unsigned int size);
static void seek (int fd, unsigned position);
static unsigned tell (int fd);
static void close (int fd);
static int fibonacci (int n);
static int max_of_four_int (int a, int b, int c, int d);

static struct lock filesys_lock;

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&filesys_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  ASSERT (f != NULL);

  int syscall_num = *(int *) validate_ptr (f->esp);
  switch (syscall_num)
    {
      case SYS_HALT:
        halt ();
        break;
      case SYS_EXIT:
        exit (*(int *) validate_ptr (f->esp + 4));
        break;
      case SYS_EXEC:
        f->eax = exec (validate_ptr (f->esp + 4));
        break;
      case SYS_WAIT:
        f->eax = wait (*(tid_t *) validate_ptr (f->esp + 4));
        break;
      case SYS_CREATE:
        f->eax = create (validate_ptr (f->esp + 4),
                         *(unsigned int *) validate_ptr (f->esp + 8));
        break;
      case SYS_REMOVE:
        f->eax = remove (validate_ptr (f->esp + 4));
        break;
      case SYS_OPEN:
        f->eax = open (validate_ptr (f->esp + 4));
        break;
      case SYS_FILESIZE:
        f->eax = filesize (*(int *) validate_ptr (f->esp + 4));
        break;
      case SYS_READ:
        f->eax = read (*(int *) validate_ptr (f->esp + 4),
                       validate_ptr (f->esp + 8),
                       *(unsigned int *) validate_ptr (f->esp + 12));
        break;
      case SYS_WRITE:
        f->eax = write (*(int *) validate_ptr (f->esp + 4),
                        validate_ptr (f->esp + 8),
                        *(unsigned int *) validate_ptr (f->esp + 12));
        break;
      case SYS_SEEK:
        seek (*(int *) validate_ptr (f->esp + 4),
              *(unsigned int *) validate_ptr (f->esp + 8));
        break;
      case SYS_TELL:
        f->eax = tell (*(int *) validate_ptr (f->esp + 4));
        break;
      case SYS_CLOSE:
        close (*(int *) validate_ptr (f->esp + 4));
        break;
      case SYS_FIBONACCI:
        f->eax = fibonacci (*(int *) validate_ptr (f->esp + 4));
        break;
      case SYS_MAXOFFOURINT:
        f->eax = max_of_four_int (*(int *) validate_ptr (f->esp + 4),
                                  *(int *) validate_ptr (f->esp + 8),
                                  *(int *) validate_ptr (f->esp + 12),
                                  *(int *) validate_ptr (f->esp + 16));
        break;
      default:
        /* Invalid system call number. Terminate current process. */
        exit (-1);
        break;
    }
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault occurred. */
static int
get_user (const uint8_t *uaddr)
{
  ASSERT (uaddr != NULL);

  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
      : "=&a" (result) : "m" (*uaddr));

  return result;
}

/* Replace UINDIRECT with the indirected user virtual address UPTR.
   In order to do that, UINDIRECT must be a pointer to a void * typed variable,
   which is used to indirect address accessing.
   If UPTR is invalid, call EXIT (-1) and terminate current process. */
static void
indirect_user (const void *uptr, void *uindirect)
{
  ASSERT (uptr != NULL);
  ASSERT (uindirect != NULL);

  for (size_t i = 0; i < sizeof uptr; ++i)
    {
      int value = get_user (uptr + i);
      if (value == -1)
        exit (-1);

      *(char *) (uindirect + i) = value & 0xFF;
    }
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  ASSERT (udst != NULL);

  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
      : "=&a" (error_code), "=m" (*udst) : "q" (byte));

  return error_code != -1;
}

/* Return PTR if it is not NULL and points to a valid user virtual address.
   Otherwise, call EXIT (-1) and terminate current process. */
static void *
validate_ptr (void *ptr)
{
  if (ptr == NULL)
    exit (-1);

  if (!is_user_vaddr (ptr))
    exit (-1);

  for (size_t i = 0; i < sizeof ptr; ++i)
    if (get_user (ptr + i) == -1)
      exit (-1);

  return ptr;
}

/* Returns a file descriptor to use for a new file. */
static int
allocate_fd (void)
{
  static int next_fd = 2; // 0 is STDIN_FILENO and 1 is STDOUT_FILENO.
  int fd;

  lock_acquire (&filesys_lock);
  fd = next_fd++;
  lock_release (&filesys_lock);

  return fd;
}
/* Halt the operating system. */
static void
halt (void)
{
  shutdown_power_off ();
}

/* Terminate this process. */
static void
exit (int status)
{
  if (lock_held_by_current_thread (&filesys_lock))
    lock_release (&filesys_lock);

  thread_current ()->pcb->exit_status = status;
  thread_exit ();
}

/* Start another process. */
static tid_t
exec (const char *task)
{
  ASSERT (task != NULL);

  void *buffer_indirect;
  indirect_user (task, &buffer_indirect);

  validate_ptr (buffer_indirect);

  return process_execute (buffer_indirect);
}

/* Wait for a child process to die. */
static int
wait (tid_t tid)
{
  ASSERT (tid >= 1);

  return process_wait (tid);
}

/* Create a file. */
bool
create (const char *filename, unsigned initial_size)
{
  ASSERT (filename != NULL);

  void *filename_indirect;
  indirect_user (filename, &filename_indirect);

  validate_ptr (filename_indirect);

  lock_acquire (&filesys_lock);
  bool success = filesys_create (filename_indirect, initial_size);
  lock_release (&filesys_lock);

  return success;
}

/* Delete a file. */
bool
remove (const char *filename)
{
  // TODO: Implement.
  ASSERT (filename != NULL);
}

/* Open a file. */
int
open (const char *filename)
{
  ASSERT (filename != NULL);

  void *filename_indirect;
  indirect_user (filename, &filename_indirect);

  validate_ptr (filename_indirect);

  lock_acquire (&filesys_lock);
  struct file *file = filesys_open (filename_indirect);
  lock_release (&filesys_lock);

  if (file == NULL)
    return -1;

  file->fd = allocate_fd ();
  list_push_back (&thread_current ()->files, file);

  return file->fd;
}

/* Obtain a file's size. */
int
filesize (int fd)
{
  // TODO: Implement.
}

/* Read from a file. */
static int
read (int fd, void *buffer, unsigned int size)
{
  ASSERT (buffer != NULL);

  if (fd == STDIN_FILENO)
    {
      for (unsigned int i = 0; i < size; ++i)
        if (!put_user (buffer + i, input_getc ()))
          exit (-1);

      return size;
    }

  return -1;
}

/* Write to a file. */
static int
write (int fd, const void *buffer, unsigned int size)
{
  ASSERT (buffer != NULL);

  void *buffer_indirect;
  indirect_user (buffer, &buffer_indirect);

  validate_ptr (buffer_indirect);

  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer_indirect, size);
      return size;
    }

  return -1;
}

/* Change position in a file. */
void
seek (int fd, unsigned position)
{
  // TODO: Implement.
}

/* Report current position in a file. */
unsigned
tell (int fd)
{
  // TODO: Implement.
}

/* Close a file. */
void
close (int fd)
{
  // TODO: Implement.
}

/* Get n-th value of Fibonacci sequence. */
static int
fibonacci (int n)
{
  int fib = n;
  int temp[2] = {0, 1};
  for (int i = 2; i <= n; ++i)
    {
      fib = temp[0] + temp[1];
      temp[0] = temp[1];
      temp[1] = fib;
    }

  return fib;
}

/* Get maximum of four integers. */
static int
max_of_four_int (int a, int b, int c, int d)
{
  int max = a;

  if (max < b)
    max = b;

  if (max < c)
    max = c;

  if (max < d)
    max = d;

  return max;
}
