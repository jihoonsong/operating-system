#include "bitmap_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

static bitmap_cmd_type convert_to_bitmap_cmd_type (const char *cmd);
static void execute_create (const int argc, const char *argv[]);
static void execute_delete (const int argc, const char *argv[]);
static void execute_dumpdata (const int argc, const char *argv[]);
static void execute_bitmap_all (const int argc, const char *argv[]);
static void execute_bitmap_contains (const int argc, const char *argv[]);
static void execute_bitmap_flip (const int argc, const char *argv[]);
static void execute_bitmap_mark (const int argc, const char *argv[]);
static void execute_bitmap_none (const int argc, const char *argv[]);
static void execute_bitmap_reset (const int argc, const char *argv[]);
static void execute_bitmap_scan_and_flip (const int argc, const char *argv[]);
static void execute_bitmap_set (const int argc, const char *argv[]);
static void execute_bitmap_set_all (const int argc, const char *argv[]);
static void execute_bitmap_set_multiple (const int argc, const char *argv[]);
static void execute_bitmap_size (const int argc, const char *argv[]);
static void execute_bitmap_test (const int argc, const char *argv[]);

/* TODO: Complete document. */
void
bitmap_handler_invoke (const char *cmd, const int argc,
                       const char *argv[])
{
  printf ("bitmap handler is invoked\n");
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
execute_bitmap_all (const int argc, const char *argv[])
{
  printf ("execute_bitmap_all\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_contains (const int argc, const char *argv[])
{
  printf ("execute_bitmap_contains\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_flip (const int argc, const char *argv[])
{
  printf ("execute_bitmap_flip\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_mark (const int argc, const char *argv[])
{
  printf ("execute_bitmap_mark\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_none (const int argc, const char *argv[])
{
  printf ("execute_bitmap_none\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_reset (const int argc, const char *argv[])
{
  printf ("execute_bitmap_reset\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_scan_and_flip (const int argc, const char *argv[])
{
  printf ("execute_bitmap_scan_and_flip\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_set (const int argc, const char *argv[])
{
  printf ("execute_bitmap_set\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_set_all (const int argc, const char *argv[])
{
  printf ("execute_bitmap_set_all\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_set_multiple (const int argc, const char *argv[])
{
  printf ("execute_bitmap_set_multiple\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_size (const int argc, const char *argv[])
{
  printf ("execute_bitmap_size\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_test (const int argc, const char *argv[])
{
  printf ("execute_bitmap_test\n");
}
