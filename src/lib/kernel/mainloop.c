#include "mainloop.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdbool.h>
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define INPUT_LEN 80

static bool receive_input (char *input);

/* The input is tokenized into command and arguments. After tokenizing is
   done, a corresponding handler is designated. */
struct command
  {
    /* A command to be executed. */
    char *cmd;
    /* The number of arguments. */
    int argc;
    /* A NULL-terminated list of arguments. */
    char *argv[ARGC_MAX + 1];
    /* A designated handler. */
    void (*handler)(const char *, const int, const char *[]);
  };

/* Call all initializers that need to be invoked before mainloop begins. */
void
mainloop_initialize (void)
{
}

/* Release memory. */
void
mainloop_terminate (void)
{
}

/* Start mainloop. */
void
mainloop_launch (void)
{
  char input[INPUT_LEN] = {0,};
  struct command command = {0,};

  while (true)
    {
      if (receive_input (input))
        {
          printf ("%s", input);
        }
    }
}

/* Reads stream data from stdin and stores it in INPUT.
   Returns true if read succeeds, false otherwise. */
static bool
receive_input (char *input)
{
  ASSERT (input != NULL);

  return fgets (input, INPUT_LEN, stdin) != NULL;
}
