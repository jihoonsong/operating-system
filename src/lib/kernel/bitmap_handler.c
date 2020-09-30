#include "bitmap_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

/* Bitmap command. */
typedef enum bitmap_cmd_type {CREATE, DELETE, DUMPDATA, BITMAP_ALL,
                              BITMAP_CONTAINS, BITMAP_FLIP, BITMAP_MARK,
                              BITMAP_NONE, BITMAP_RESET, BITMAP_SCAN_AND_FLIP,
                              BITMAP_SET, BITMAP_SET_ALL, BITMAP_SET_MULTIPLE,
                              BITMAP_SIZE, BITMAP_TEST,
                              BITMAP_CMD_COUNT, NONE} bitmap_cmd_type;
typedef void (*bitmap_cmd_ptr) (const int, const char *[]);
struct bitmap_cmd_table
  {
    const bitmap_cmd_type type;
    const char *name;
    const bitmap_cmd_ptr execute;
  };

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

/* Bitmap command table. */
static const struct bitmap_cmd_table bitmap_cmd_table[BITMAP_CMD_COUNT] = \
  {{CREATE, "create", execute_create},
   {DELETE, "delete", execute_delete},
   {DUMPDATA, "dumpdata", execute_dumpdata},
   {BITMAP_ALL, "bitmap_all", execute_bitmap_all},
   {BITMAP_CONTAINS, "bitmap_contains", execute_bitmap_contains},
   {BITMAP_FLIP, "bitmap_flip", execute_bitmap_flip},
   {BITMAP_MARK, "bitmap_mark", execute_bitmap_mark},
   {BITMAP_NONE, "bitmap_none", execute_bitmap_none},
   {BITMAP_RESET, "bitmap_reset", execute_bitmap_reset},
   {BITMAP_SCAN_AND_FLIP, "bitmap_scan_and_flip", execute_bitmap_scan_and_flip},
   {BITMAP_SET, "bitmap_set", execute_bitmap_set},
   {BITMAP_SET_ALL, "bitmap_set_all", execute_bitmap_set_all},
   {BITMAP_SET_MULTIPLE, "bitmap_set_multiple", execute_bitmap_set_multiple},
   {BITMAP_SIZE, "bitmap_size", execute_bitmap_size},
   {BITMAP_TEST, "bitmap_test", execute_bitmap_test}};

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
