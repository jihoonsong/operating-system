#include "mainloop.h"

/* Initialize and launch mainloop. Call termination after mainloop is over. */
int
main (void)
{
  mainloop_initialize();
  mainloop_launch();
  mainloop_terminate();

  return 0;
}
