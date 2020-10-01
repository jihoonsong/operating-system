#include "list_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>
#include <string.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

/* List command. */
typedef enum list_cmd_type {CREATE, DELETE, DUMPDATA, LIST_BACK, LIST_EMPTY,
                            LIST_FRONT, LIST_INSERT, LIST_INSERT_ORDERED,
                            LIST_MAX, LIST_MIN, LIST_POP_BACK, LIST_POP_FRONT,
                            LIST_PUSH_BACK, LIST_PUSH_FRONT, LIST_REMOVE,
                            LIST_REVERSE, LIST_SHUFFLE, LIST_SIZE, LIST_SORT,
                            LIST_SPLICE, LIST_SWAP, LIST_UNIQUE,
                            LIST_CMD_COUNT, NONE} list_cmd_type;
typedef void (*list_cmd_ptr) (const int, const char *[]);
struct list_cmd_table
  {
    const list_cmd_type type;
    const char *name;
    const list_cmd_ptr execute;
  };

static list_cmd_type convert_to_list_cmd_type (const char *cmd);
static void execute_create (const int argc, const char *argv[]);
static void execute_delete (const int argc, const char *argv[]);
static void execute_dumpdata (const int argc, const char *argv[]);
static void execute_list_back (const int argc, const char *argv[]);
static void execute_list_empty (const int argc, const char *argv[]);
static void execute_list_front (const int argc, const char *argv[]);
static void execute_list_insert (const int argc, const char *argv[]);
static void execute_list_insert_ordered (const int argc, const char *argv[]);
static void execute_list_max (const int argc, const char *argv[]);
static void execute_list_min (const int argc, const char *argv[]);
static void execute_list_pop_back (const int argc, const char *argv[]);
static void execute_list_pop_front (const int argc, const char *argv[]);
static void execute_list_push_back (const int argc, const char *argv[]);
static void execute_list_push_front (const int argc, const char *argv[]);
static void execute_list_remove (const int argc, const char *argv[]);
static void execute_list_reverse (const int argc, const char *argv[]);
static void execute_list_shuffle (const int argc, const char *argv[]);
static void execute_list_size (const int argc, const char *argv[]);
static void execute_list_sort (const int argc, const char *argv[]);
static void execute_list_splice (const int argc, const char *argv[]);
static void execute_list_swap (const int argc, const char *argv[]);
static void execute_list_unique (const int argc, const char *argv[]);

/* List command table. */
static const struct list_cmd_table list_cmd_table[LIST_CMD_COUNT] = \
  {{CREATE, "create", execute_create},
   {DELETE, "delete", execute_delete},
   {DUMPDATA, "dumpdata", execute_dumpdata},
   {LIST_BACK, "list_back", execute_list_back},
   {LIST_EMPTY, "list_empty", execute_list_empty},
   {LIST_FRONT, "list_front", execute_list_front},
   {LIST_INSERT, "list_insert", execute_list_insert},
   {LIST_INSERT_ORDERED, "list_insert_ordered", execute_list_insert_ordered},
   {LIST_MAX, "list_max", execute_list_max},
   {LIST_MIN, "list_min", execute_list_min},
   {LIST_POP_BACK, "list_pop_back", execute_list_pop_back},
   {LIST_POP_FRONT, "list_pop_front", execute_list_pop_front},
   {LIST_PUSH_BACK, "list_push_back", execute_list_push_back},
   {LIST_PUSH_FRONT, "list_push_front", execute_list_push_front},
   {LIST_REMOVE, "list_remove", execute_list_remove},
   {LIST_REVERSE, "list_reverse", execute_list_reverse},
   {LIST_SHUFFLE, "list_shuffle", execute_list_shuffle},
   {LIST_SIZE, "list_size", execute_list_size},
   {LIST_SORT, "list_sort", execute_list_sort},
   {LIST_SPLICE, "list_splice", execute_list_splice},
   {LIST_SWAP, "list_swap", execute_list_swap},
   {LIST_UNIQUE, "list_unique", execute_list_unique}};

/* Initializes list table. */
void
list_handler_initialize (void)
{
}

/* Releases memory. */
void
list_handler_terminate (void)
{
}

/* Executes CMD. */
void
list_handler_invoke (const char *cmd, const int argc,
                     const char *argv[])
{
  ASSERT (cmd != NULL);

  list_cmd_type type = convert_to_list_cmd_type (cmd);
  if (type != NONE)
    list_cmd_table[type].execute (argc, argv);
}

/* Converts CMD to its corresponding list command type.
   Returns its list command type if conversion succeeds, NONE otherwise. */
static list_cmd_type
convert_to_list_cmd_type (const char *cmd)
{
  ASSERT (cmd != NULL);

  for (int i = 0; i < LIST_CMD_COUNT; ++i)
    if (strcmp(cmd, list_cmd_table[i].name) == 0)
      return list_cmd_table[i].type;

  printf ("%s: command not found\n", cmd);
  return NONE;
}

/* TODO: Complete document. */
static void
execute_create (const int argc, const char *argv[])
{
  printf ("execute_create\n");
}

/* TODO: Complete document. */
static void
execute_delete (const int argc, const char *argv[])
{
  printf ("execute_delete\n");
}

/* TODO: Complete document. */
static void
execute_dumpdata (const int argc, const char *argv[])
{
  printf ("execute_dumpdata\n");
}

/* TODO: Complete document. */
static void
execute_list_back (const int argc, const char *argv[])
{
  printf ("execute_list_back\n");
}

/* TODO: Complete document. */
static void
execute_list_empty (const int argc, const char *argv[])
{
  printf ("execute_list_empty\n");
}

/* TODO: Complete document. */
static void
execute_list_front (const int argc, const char *argv[])
{
  printf ("execute_list_front\n");
}

/* TODO: Complete document. */
static void
execute_list_insert (const int argc, const char *argv[])
{
  printf ("execute_list_insert\n");
}

/* TODO: Complete document. */
static void
execute_list_insert_ordered (const int argc, const char *argv[])
{
  printf ("execute_list_insert_ordered\n");
}

/* TODO: Complete document. */
static void
execute_list_max (const int argc, const char *argv[])
{
  printf ("execute_list_max\n");
}

/* TODO: Complete document. */
static void
execute_list_min (const int argc, const char *argv[])
{
  printf ("execute_list_min\n");
}

/* TODO: Complete document. */
static void
execute_list_pop_back (const int argc, const char *argv[])
{
  printf ("execute_list_pop_back\n");
}

/* TODO: Complete document. */
static void
execute_list_pop_front (const int argc, const char *argv[])
{
  printf ("execute_list_pop_front\n");
}

/* TODO: Complete document. */
static void
execute_list_push_back (const int argc, const char *argv[])
{
  printf ("execute_list_push_back\n");
}

/* TODO: Complete document. */
static void
execute_list_push_front (const int argc, const char *argv[])
{
  printf ("execute_list_push_front\n");
}

/* TODO: Complete document. */
static void
execute_list_remove (const int argc, const char *argv[])
{
  printf ("execute_list_remove\n");
}

/* TODO: Complete document. */
static void
execute_list_reverse (const int argc, const char *argv[])
{
  printf ("execute_list_reverse\n");
}

/* TODO: Complete document. */
static void
execute_list_shuffle (const int argc, const char *argv[])
{
  printf ("execute_list_shuffle\n");
}

/* TODO: Complete document. */
static void
execute_list_size (const int argc, const char *argv[])
{
  printf ("execute_list_size\n");
}

/* TODO: Complete document. */
static void
execute_list_sort (const int argc, const char *argv[])
{
  printf ("execute_list_sort\n");
}

/* TODO: Complete document. */
static void
execute_list_splice (const int argc, const char *argv[])
{
  printf ("execute_list_splice\n");
}

/* TODO: Complete document. */
static void
execute_list_swap (const int argc, const char *argv[])
{
  printf ("execute_list_swap\n");
}

/* TODO: Complete document. */
static void
execute_list_unique (const int argc, const char *argv[])
{
  printf ("execute_list_unique\n");
}
