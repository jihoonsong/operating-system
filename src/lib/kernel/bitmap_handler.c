#include "bitmap_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define DECIMAL 10
#define MAX_BITMAP_COUNT 10
#define MAX_BITMAP_NAME 100

/* Bitmap command. */
typedef enum bitmap_cmd_type {CREATE, DELETE, DUMPDATA, BITMAP_ALL, BITMAP_ANY,
                              BITMAP_CONTAINS, BITMAP_COUNT, BITMAP_DUMP,
                              BITMAP_EXPAND, BITMAP_FLIP, BITMAP_MARK,
                              BITMAP_NONE, BITMAP_RESET, BITMAP_SCAN,
                              BITMAP_SCAN_AND_FLIP, BITMAP_SET, BITMAP_SET_ALL,
                              BITMAP_SET_MULTIPLE, BITMAP_SIZE, BITMAP_TEST,
                              BITMAP_CMD_COUNT, NONE} bitmap_cmd_type;
typedef void (*bitmap_cmd_ptr) (const int, const char *[]);
struct bitmap_cmd_table
  {
    const bitmap_cmd_type type;
    const char *name;
    const bitmap_cmd_ptr execute;
  };

/* Execution functions. */
static bitmap_cmd_type convert_to_bitmap_cmd_type (const char *cmd);
static void execute_create (const int argc, const char *argv[]);
static void execute_delete (const int argc, const char *argv[]);
static void execute_dumpdata (const int argc, const char *argv[]);
static void execute_bitmap_all (const int argc, const char *argv[]);
static void execute_bitmap_any (const int argc, const char *argv[]);
static void execute_bitmap_contains (const int argc, const char *argv[]);
static void execute_bitmap_count (const int argc, const char *argv[]);
static void execute_bitmap_expand (const int argc, const char *argv[]);
static void execute_bitmap_dump (const int argc, const char *argv[]);
static void execute_bitmap_flip (const int argc, const char *argv[]);
static void execute_bitmap_mark (const int argc, const char *argv[]);
static void execute_bitmap_none (const int argc, const char *argv[]);
static void execute_bitmap_reset (const int argc, const char *argv[]);
static void execute_bitmap_scan (const int argc, const char *argv[]);
static void execute_bitmap_scan_and_flip (const int argc, const char *argv[]);
static void execute_bitmap_set (const int argc, const char *argv[]);
static void execute_bitmap_set_all (const int argc, const char *argv[]);
static void execute_bitmap_set_multiple (const int argc, const char *argv[]);
static void execute_bitmap_size (const int argc, const char *argv[]);
static void execute_bitmap_test (const int argc, const char *argv[]);

/* Bitmap table functions. */
static struct bitmap_table *find_bitmap_table_entry (const char *arg);
static struct bitmap_table *get_empty_bitmap_table_entry (void);

/* Bitmap command table. */
static const struct bitmap_cmd_table bitmap_cmd_table[BITMAP_CMD_COUNT] = \
  {{CREATE, "create", execute_create},
   {DELETE, "delete", execute_delete},
   {DUMPDATA, "dumpdata", execute_dumpdata},
   {BITMAP_ALL, "bitmap_all", execute_bitmap_all},
   {BITMAP_ANY, "bitmap_any", execute_bitmap_any},
   {BITMAP_CONTAINS, "bitmap_contains", execute_bitmap_contains},
   {BITMAP_COUNT, "bitmap_count", execute_bitmap_count},
   {BITMAP_DUMP, "bitmap_dump", execute_bitmap_dump},
   {BITMAP_EXPAND, "bitmap_expand", execute_bitmap_expand},
   {BITMAP_FLIP, "bitmap_flip", execute_bitmap_flip},
   {BITMAP_MARK, "bitmap_mark", execute_bitmap_mark},
   {BITMAP_NONE, "bitmap_none", execute_bitmap_none},
   {BITMAP_RESET, "bitmap_reset", execute_bitmap_reset},
   {BITMAP_SCAN, "bitmap_scan", execute_bitmap_scan},
   {BITMAP_SCAN_AND_FLIP, "bitmap_scan_and_flip", execute_bitmap_scan_and_flip},
   {BITMAP_SET, "bitmap_set", execute_bitmap_set},
   {BITMAP_SET_ALL, "bitmap_set_all", execute_bitmap_set_all},
   {BITMAP_SET_MULTIPLE, "bitmap_set_multiple", execute_bitmap_set_multiple},
   {BITMAP_SIZE, "bitmap_size", execute_bitmap_size},
   {BITMAP_TEST, "bitmap_test", execute_bitmap_test}};

/* Bitmap table. */
struct bitmap_table
  {
    char name[MAX_BITMAP_NAME];
    struct bitmap *bitmap;
  };
static struct bitmap_table bitmap_table[MAX_BITMAP_COUNT];

/* Returns true if bitmap table is full, false otherwise. */
static inline bool
is_bitmap_table_full (void)
{
  for (int i = 0; i < MAX_BITMAP_COUNT; ++i)
    if (bitmap_table[i].name[0] == '\0')
      return false;

  return true;
}

/* Initializes bitmap table. */
void
bitmap_handler_initialize (void)
{
  memset (bitmap_table, '\0', sizeof bitmap_table);
}

/* Releases memory. */
void
bitmap_handler_terminate (void)
{
  // TODO: Release bitmap_table.
}

/* Executes CMD. */
void
bitmap_handler_invoke (const char *cmd, const int argc,
                       const char *argv[])
{
  ASSERT (cmd != NULL);

  bitmap_cmd_type type = convert_to_bitmap_cmd_type(cmd);
  if (type != NONE)
    bitmap_cmd_table[type].execute (argc, argv);
}

/* Converts CMD to its corresponding bitmap command type.
   Returns its list command type if conversion succeeds, NONE otherwise. */
static bitmap_cmd_type
convert_to_bitmap_cmd_type (const char *cmd)
{
  ASSERT (cmd != NULL);

  for (int i = 0; i < BITMAP_CMD_COUNT; ++i)
    if (strcmp (cmd, bitmap_cmd_table[i].name) == 0)
      return bitmap_cmd_table[i].type;

  printf ("%s: command not found\n", cmd);
  return NONE;
}

/* Creates a new bitmap of ARGV[2] size with the name of ARGV[1]. */
static void
execute_create (const int argc, const char *argv[])
{
  ASSERT (argc == 3);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (argv[2] != NULL);

  if (find_bitmap_table_entry (argv[1]) != NULL)
    {
      printf ("%s: already exists\n", argv[1]);
      return;
    }

  if (is_bitmap_table_full ())
    {
      printf ("Bitmap table is full\n");
      return;
    }

  char *endptr = NULL;
  int bit_cnt = strtol (argv[2], &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", argv[2]);
      return;
    }

  struct bitmap *new_bitmap = bitmap_create (bit_cnt);
  if (new_bitmap == NULL)
    {
      printf ("Failed to create bitmap.\n");
      return;
    }

  struct bitmap_table *new_entry = get_empty_bitmap_table_entry ();
  memcpy (new_entry->name, argv[1], strlen (argv[1]) + 1);
  new_entry->bitmap = new_bitmap;
}

/* TODO: Complete document. */
static void
execute_delete (const int argc, const char *argv[])
{
  printf ("execute_delete\n");
}

/* Prints the contents of a bitmap with a name of ARGV[0]. */
static void
execute_dumpdata (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    return;

  for (int i = 0; i < bitmap_size (entry->bitmap); ++i)
    printf (bitmap_test (entry->bitmap, i) ? "1" : "0");

  printf ("\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_all (const int argc, const char *argv[])
{
  printf ("execute_bitmap_all\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_any (const int argc, const char *argv[])
{
  printf ("execute_bitmap_any\n");
}

/* Returns true if any bits in a bitmap with the name of ARGV[0]
   between ARGV[1] and ARGV[1] + ARGV[2],  exclusive, are set to ARGV[3],
   and false otherwise. */
static void
execute_bitmap_contains (const int argc, const char *argv[])
{
  printf ("execute_bitmap_contains\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_count (const int argc, const char *argv[])
{
  printf ("execute_bitmap_count\n");
}

/* TODO: Complete document. */
static void
execute_bitmap_expand (const int argc, const char *argv[])
{
  printf ("execute_bitmap_expand\n");
}

/* Dumps the contents of a bitmap with a name of ARGV[0]. */
static void execute_bitmap_dump (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: bitmap not found\n", argv[0]);
      return;
    }

  bitmap_dump (entry->bitmap);
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
execute_bitmap_scan (const int argc, const char *argv[])
{
  printf ("execute_bitmap_scan\n");
}

/* Finds the first group of ARGV[2] consecutive bits in a bitmap with
   the name of ARGV[0] at or after ARGV[1] that are all set to ARGV[3],
   flips them all to !ARGV[3], and returns the index of the first bit
   in the group.
   If there is no such group, returns BITMAP_ERROR.
   If ARGV[2] is zero, returns 0.
   Bits are set atomically, but testing bits is not atomic with
   setting them. */
static void
execute_bitmap_scan_and_flip (const int argc, const char *argv[])
{
  printf ("execute_bitmap_scan_and_flip\n");
}

/* Sets the bit numbered ARGV[1] in a bitmap with the name of ARGV[0]
   to ARGV[2]. */
static void
execute_bitmap_set (const int argc, const char *argv[])
{
  ASSERT (argc == 3);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (argv[2] != NULL);
  ASSERT (strcmp (argv[2], "true") == 0 || strcmp (argv[2], "false") == 0);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: bitmap not found\n", argv[0]);
      return;
    }

  char *endptr = NULL;
  int idx = strtol (argv[1], &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", argv[1]);
      return;
    }

  bool value = strcmp (argv[2], "true") == 0 ? true : false;

  bitmap_set (entry->bitmap, idx, value);
}

/* Sets all bits in a bitmap with the name of ARGV[0] to ARGV[2]. */
static void
execute_bitmap_set_all (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (strcmp (argv[1], "true") == 0 || strcmp (argv[1], "false") == 0);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: bitmap not found\n", argv[0]);
      return;
    }

  bool value = strcmp(argv[1], "true") == 0 ? true : false;

  bitmap_set_all (entry->bitmap, value);
}

/* Sets the ARGV[2] bits starting at ARGV[1] in a bitmap with the name of
   ARGV[0] to ARGV[3]. */
static void
execute_bitmap_set_multiple (const int argc, const char *argv[])
{
  ASSERT (argc == 4);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (argv[2] != NULL);
  ASSERT (strcmp (argv[3], "true") == 0 || strcmp (argv[3], "false") == 0);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: bitmap not found\n", argv[0]);
      return;
    }

  char *endptr = NULL;
  int start = strtol (argv[1], &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", argv[1]);
      return;
    }

  int cnt = strtol (argv[2], &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", argv[2]);
      return;
    }

  bool value = strcmp (argv[3], "true") == 0 ? true : false;

  bitmap_set_multiple (entry->bitmap, start, cnt, value);
}

/* Returns the number of bits in a bitmap with the name of ARGV[0]. */
static void
execute_bitmap_size (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: bitmap not found\n", argv[0]);
      return;
    }

  printf ("%zu\n", bitmap_size (entry->bitmap));
}

/* Returns the value of the bit numbered ARGV[1] in a bitmap with
   the name of ARGV[0]. */
static void
execute_bitmap_test (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct bitmap_table *entry = find_bitmap_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: bitmap not found\n", argv[0]);
      return;
    }

  char *endptr = NULL;
  int idx = strtol (argv[1], &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", argv[1]);
      return;
    }

  printf (bitmap_test (entry->bitmap, idx) ? "true\n" : "false\n");
}

/* Finds a bitmap table entry that has the same name as ARG.
   Returns a pointer to the bitmap table entry if search succeeds,
   NULL otherwise. */
static struct bitmap_table *
find_bitmap_table_entry (const char *arg)
{
  ASSERT (arg != NULL);

  for (int i = 0; i < MAX_BITMAP_COUNT; ++i)
    if (strcmp (arg, bitmap_table[i].name) == 0)
      return &bitmap_table[i];

  return NULL;
}

/* Returns an empty bitmap table entry, NULL if bitmap table is full. */
static struct bitmap_table *
get_empty_bitmap_table_entry (void)
{
  for (int i = 0; i < MAX_BITMAP_COUNT; ++i)
      if (bitmap_table[i].name[0] == '\0')
        return &bitmap_table[i];

  return NULL;
}
