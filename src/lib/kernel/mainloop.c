#include "mainloop.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "bitmap_handler.h"
#include "hash_handler.h"
#include "list_handler.h"

#define ARGC_MAX 4
#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define INPUT_LEN 80

/* Handler. */
typedef enum handler_type {LIST, HASH, BITMAP,
                           HANDLER_COUNT, NONE} handler_type;
typedef void (*handler_ptr) (const char *, const int, const char *[]);

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
    handler_type handler;
  };

static void broadcast_to_handlers (const struct command *command,
                                   const handler_ptr *handler_ptrs);
static handler_type convert_to_handler_type (const char *argv,
                                             const char *handler_specifiers[]);
static bool designate_handler (struct command *command,
                               char *handler_prefixes[],
                               char *handler_specifiers[]);
static void initialize_handlers (handler_ptr *handler_ptrs,
                                 char *handler_prefixes[],
                                 char *handler_specifiers[]);
static void initialize_handler_ptrs (handler_ptr *handler_ptrs);
static void initialize_handler_prefixes (char *handler_prefixes[]);
static void initialize_handler_specifiers (char *handler_specifiers[]);
static void invoke_handler (const struct command *command,
                            const handler_ptr *handler_ptrs);
static bool receive_input (char *input);
static void tokenize (char *input, struct command *command);

/* Returns true if CMD equals to "dumpdata" or "delete", false otherwise. */
static inline bool
is_broadcast (const char *cmd)
{
  ASSERT (cmd != NULL);

  return strcmp (cmd, "dumpdata") == 0 || strcmp (cmd, "delete") == 0;
}

/* Returns true if CMD equals "quit", false otherwise. */
static inline bool
is_quit (const char *cmd)
{
  ASSERT (cmd != NULL);

  return strcmp (cmd, "quit") == 0;
}

/* Call all initializers that need to be invoked before mainloop begins. */
void
mainloop_initialize (void)
{
  list_handler_initialize ();
}

/* Release memory. */
void
mainloop_terminate (void)
{
  list_handler_terminate ();
}

/* Start mainloop. */
void
mainloop_launch (void)
{
  char input[INPUT_LEN] = {0,};
  struct command command = {0,};
  handler_ptr handler_ptrs[HANDLER_COUNT] = {NULL,};
  char *handler_prefixes[HANDLER_COUNT] = {NULL,};
  char *handler_specifiers[HANDLER_COUNT] = {NULL,};

  initialize_handlers (handler_ptrs, handler_prefixes, handler_specifiers);

  while (true)
    {
      if (receive_input (input))
        {
          tokenize (input, &command);

          if (is_quit (command.cmd))
            break;

          if (is_broadcast (command.cmd))
            {
              broadcast_to_handlers (&command, handler_ptrs);
              continue;
            }

          if (designate_handler (&command, handler_prefixes,
                                 handler_specifiers))
              invoke_handler (&command, handler_ptrs);
        }
    }
}

/* Broadcast to all handlers. */
static void
broadcast_to_handlers (const struct command *command,
                       const handler_ptr *handler_ptrs)
{
  ASSERT (command != NULL);
  ASSERT (handler_ptrs != NULL);

  for (int i = 0; i < HANDLER_COUNT; ++i)
    handler_ptrs[i]( (const char *)command->cmd,
                     (const int)command->argc,
                     (const char **)command->argv);
}

/* Convert ARGV to the type of its designated handler. */
static handler_type
convert_to_handler_type (const char *argv,
                         const char *handler_specifiers[])
{
  ASSERT (handler_specifiers != NULL);

  if (argv == NULL)
    {
      printf ("create: missing operand\n");
      return NONE;
    }

  if (strcmp (argv, handler_specifiers[LIST]) == 0)
    {
      return LIST;
    }
  if (strcmp (argv, handler_specifiers[HASH]) == 0)
    {
      return HASH;
    }
  if (strcmp (argv, handler_specifiers[BITMAP]) == 0)
    {
      return BITMAP;
    }

  printf ("create: invalid operand '%s'\n", argv);
  return NONE;
}

/* Designate handler corresponding to the COMMAND->CMD.
   Returns true if handler designation succeeds, false otherwise. */
static bool
designate_handler (struct command *command,
                   char *handler_prefixes[],
                   char *handler_specifiers[])
{
  ASSERT (command != NULL);
  ASSERT (handler_prefixes != NULL);
  ASSERT (handler_specifiers != NULL);

  if (strcmp (command->cmd, "create") == 0)
    {
      command->handler = \
        convert_to_handler_type((const char *) command->argv[0],
                                (const char **) handler_specifiers);
      return command->handler != NONE;
    }

  if (strncmp (command->cmd, handler_prefixes[LIST],
               strlen(handler_prefixes[LIST])) == 0)
    {
      command->handler = LIST;
    }
  else if (strncmp (command->cmd, handler_prefixes[HASH],
                    strlen(handler_prefixes[LIST])) == 0)
    {
      command->handler = HASH;
    }
  else if (strncmp (command->cmd, handler_prefixes[BITMAP],
                    strlen(handler_prefixes[BITMAP])) == 0)
    {
      command->handler = BITMAP;
    }
  else
    {
      printf ("%s: command not found\n", command->cmd);
      command->handler = NONE;
    }

  return command->handler != NONE;
}

/* Initialize handlers. */
static void
initialize_handlers (handler_ptr *handler_ptrs,
                     char *handler_prefixes[],
                     char *handler_specifiers[])
{
  ASSERT (handler_ptrs != NULL);
  ASSERT (handler_prefixes != NULL);
  ASSERT (handler_specifiers != NULL);

  initialize_handler_ptrs (handler_ptrs);
  initialize_handler_prefixes (handler_prefixes);
  initialize_handler_specifiers (handler_specifiers);
}

/* Initialize function pointers to handler. */
static void
initialize_handler_ptrs (handler_ptr *handler_ptrs)
{
  ASSERT (handler_ptrs);

  handler_ptrs[LIST] = list_handler_invoke;
  handler_ptrs[HASH] = hash_handler_invoke;
  handler_ptrs[BITMAP] = bitmap_handler_invoke;
}

/* Initialize handler prefixes. */
static void
initialize_handler_prefixes (char *handler_prefixes[])
{
  ASSERT (handler_prefixes);

  handler_prefixes[LIST] = "list_";
  handler_prefixes[HASH] = "hash_";
  handler_prefixes[BITMAP] = "bitmap_";
}

/* Initialize handler specifiers. */
static void
initialize_handler_specifiers (char *handler_specifiers[])
{
  ASSERT (handler_specifiers);

  handler_specifiers[LIST] = "list";
  handler_specifiers[HASH] = "hashtable";
  handler_specifiers[BITMAP] = "bitmap";
}

/* Invoke the designated handler. */
static void
invoke_handler (const struct command *command,
                const handler_ptr *handler_ptrs)
{
  ASSERT (command != NULL);
  ASSERT (command->handler != NONE);
  ASSERT (handler_ptrs != NULL);

  handler_ptrs[command->handler] ((const char *)command->cmd,
                                  (const int)command->argc,
                                  (const char **)command->argv);
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
