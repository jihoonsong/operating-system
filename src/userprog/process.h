#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/synch.h"
#include "threads/thread.h"

/* Process identifier. */
typedef int pid_t;
#define PID_ERROR ((pid_t) -1)

/* A process control block. */
struct process
  {
    /* Owned by process.c. */
    pid_t pid;              /* Process identifier. */
    bool alive;             /* True if exited, false otherwise. */
    bool orphan;            /* True if orphan, false otherwise. */
    bool being_waited;      /* True if wait is called on, false otherwise. */
    bool start_success;     /* True if started successfully, false otherwise. */
    int exit_status;        /* Exit status. */
    struct semaphore start; /* Semaphore to synchronize execution start. */
    struct semaphore wait;  /* Semaphore to synchronize waiting for child. */

    /* Shared between process.c and thread.c. */
    struct list_elem elem;  /* List element. */
  };

tid_t process_execute (const char *task);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
