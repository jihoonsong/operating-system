#include "mainloop.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ARGC_MAX 4
#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define INPUT_LEN 80

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

static bool receive_input (char *input);
static void tokenize (char *input, struct command *command);

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
          tokenize (input, &command);

          printf ("%s\n", command.cmd);
          printf ("%d\n", command.argc);
          for (char **argv = command.argv; *argv; ++argv)
            printf ("%s\n", *argv);
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

/* Tokenize input into command and arguments. */
static void
tokenize (char *input, struct command *command)
{
  ASSERT (input != NULL);
  ASSERT (command != NULL);

  const char delim[] = " \t";
  char *save_ptr = NULL;

  input[strlen(input) - 1] = '\0';
  command->cmd = strtok_r (input, delim, &save_ptr);
  for (command->argc = 0; command->argc < ARGC_MAX; ++command->argc)
    {
      command->argv[command->argc] = strtok_r (NULL, delim, &save_ptr);
      if (command->argv[command->argc] == NULL)
        break;
    }
}
