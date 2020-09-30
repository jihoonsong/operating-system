#ifndef __LIB_KERNEL_MAINLOOP_H
#define __LIB_KERNEL_MAINLOOP_H

/* Mainloop.

   A mainloop of the program that acts like a client of command pattern.
*/

/* Initialization and termination. */
void mainloop_initialize (void);
void mainloop_terminate (void);

/* Start mainloop. */
void mainloop_launch (void);

#endif /* lib/kernel/mainloop.h */
