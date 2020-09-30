#include "hash_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

/* Hash command. */
typedef enum hash_cmd_type {CREATE, DELETE, DUMPDATA, HASH_APPLY, HASH_CLEAR,
                            HASH_DELETE, HASH_EMPTY, HASH_FIND, HASH_INSERT,
                            HASH_REPLACE, HASH_SIZE,
                            HASH_CMD_COUNT, NONE} hash_cmd_type;
typedef void (*hash_cmd_ptr) (const int, const char *[]);
struct hash_cmd_table
  {
    const hash_cmd_type type;
    const char *name;
    const hash_cmd_ptr execute;
  };

static hash_cmd_type convert_to_hash_cmd_type (const char *cmd);
static void execute_create (const int argc, const char *argv[]);
static void execute_delete (const int argc, const char *argv[]);
static void execute_dumpdata (const int argc, const char *argv[]);
static void execute_hash_apply (const int argc, const char *argv[]);
static void execute_hash_clear (const int argc, const char *argv[]);
static void execute_hash_delete (const int argc, const char *argv[]);
static void execute_hash_empty (const int argc, const char *argv[]);
static void execute_hash_find (const int argc, const char *argv[]);
static void execute_hash_insert (const int argc, const char *argv[]);
static void execute_hash_replace (const int argc, const char *argv[]);
static void execute_hash_size (const int argc, const char *argv[]);

/* Hash command table. */
static const struct hash_cmd_table hash_cmd_table[HASH_CMD_COUNT] = \
  {{CREATE, "create", execute_create},
   {DELETE, "delete", execute_delete},
   {DUMPDATA, "dumpdata", execute_dumpdata},
   {HASH_APPLY, "hash_apply", execute_hash_apply},
   {HASH_CLEAR, "hash_clear", execute_hash_clear},
   {HASH_DELETE, "hash_delete", execute_hash_delete},
   {HASH_EMPTY, "hash_empty", execute_hash_empty},
   {HASH_FIND, "hash_find", execute_hash_find},
   {HASH_INSERT, "hash_insert", execute_hash_insert},
   {HASH_REPLACE, "hash_replace", execute_hash_replace},
   {HASH_SIZE, "hash_size", execute_hash_size}};

/* TODO: Complete document. */
void
hash_handler_invoke (const char *cmd, const int argc,
                     const char *argv[])
{
  printf ("hash handler is invoked\n");
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
execute_hash_apply (const int argc, const char *argv[])
{
  printf ("execute_hash_apply\n");
}

/* TODO: Complete document. */
static void
execute_hash_clear (const int argc, const char *argv[])
{
  printf ("execute_hash_clear\n");
}

/* TODO: Complete document. */
static void
execute_hash_delete (const int argc, const char *argv[])
{
  printf ("execute_hash_delete\n");
}

/* TODO: Complete document. */
static void
execute_hash_empty (const int argc, const char *argv[])
{
  printf ("execute_hash_empty\n");
}

/* TODO: Complete document. */
static void
execute_hash_find (const int argc, const char *argv[])
{
  printf ("execute_hash_find\n");
}

/* TODO: Complete document. */
static void
execute_hash_insert (const int argc, const char *argv[])
{
  printf ("execute_hash_insert\n");
}

/* TODO: Complete document. */
static void
execute_hash_replace (const int argc, const char *argv[])
{
  printf ("execute_hash_replace\n");
}

/* TODO: Complete document. */
static void
execute_hash_size (const int argc, const char *argv[])
{
  printf ("execute_hash_size\n");
}
