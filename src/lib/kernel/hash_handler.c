#include "hash_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

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
