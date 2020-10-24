#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "devices/input.h"
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

static int get_user (const uint8_t *uaddr);
static void indirect_user (const void *uptr, void *uindirect);
static bool put_user (uint8_t *udst, uint8_t byte);
static void *validate_ptr (void *ptr);

static void syscall_handler (struct intr_frame *);
static void halt (void);
static void exit (int status);
static tid_t exec (const char *task);
static int wait (tid_t tid);
static int read (int fd, void *buffer, unsigned int size);
static int write (int fd, const void *buffer, unsigned int size);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
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
  if (ptr == NULL || ptr >= PHYS_BASE || get_user (ptr) == -1)
    exit (-1);

  return ptr;
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
  printf ("%s: exit(%d)\n", thread_current ()->name, status);
  thread_exit ();
}

/* Start another process. */
static tid_t
exec (const char *task)
{
  ASSERT (task != NULL);

  return process_execute (task);
}

/* Wait for a child process to die. */
static int
wait (tid_t tid)
{
  ASSERT (tid >= 1);
}

/* Read from a file. */
static int
read (int fd, void *buffer, unsigned int size)
{
  ASSERT (fd > 0);
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
  ASSERT (fd > 0);
  ASSERT (buffer != NULL);

  void *buffer_indirect;
  indirect_user (buffer, &buffer_indirect);

  if (fd == STDOUT_FILENO)
    {
      putbuf (buffer_indirect, size);
      return size;
    }

  return -1;
}
