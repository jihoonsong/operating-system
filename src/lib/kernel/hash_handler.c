#include "hash_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>
#include <string.h>
#include "hash.h"

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define MAX_HASH_COUNT 10
#define MAX_HASH_NAME 100

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

/* Execution functions. */
static bool compare (const struct hash_elem *a, const struct hash_elem *b,
                     void *aux);
static unsigned hash (const struct hash_elem *e, void *aux);
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
static unsigned hash (const struct hash_elem *e, void *aux);

/* Hash table functions. */
static struct hash_table *find_hash_table_entry (const char *arg);
static struct hash_table *get_empty_hash_table_entry (void);

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

/* Hash table. */
struct hash_table
  {
    char name[MAX_HASH_NAME];
    struct hash hash;
  };
static struct hash_table hash_table[MAX_HASH_COUNT];

/* Returns true if hash table is full, false otherwise. */
static inline bool
is_hash_table_full (void)
{
  for (int i = 0; i < MAX_HASH_COUNT; ++i)
    if (hash_table[i].name[0] == '\0')
      return false;

  return true;
}

/* Initializes hash table. */
void
hash_handler_initialize (void)
{
  memset (hash_table, '\0', sizeof (hash_table));
}

/* Releases memory. */
void
hash_handler_terminate (void)
{
  // TODO: Release hash_table.
}

/* Executes CMD. */
void
hash_handler_invoke (const char *cmd, const int argc,
                     const char *argv[])
{
  ASSERT (cmd != NULL);

  hash_cmd_type type = convert_to_hash_cmd_type (cmd);
  if (type != NONE)
    hash_cmd_table[type].execute (argc, argv);
}

/* Compares the value of two list elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
static bool
compare (const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
  ASSERT (a != NULL);
  ASSERT (b != NULL);

  struct hash_item *item_a = hash_entry (a, struct hash_item, elem);
  struct hash_item *item_b = hash_entry (b, struct hash_item, elem);

  return item_a->data < item_b->data ? true : false;
}

/* Converts CMD to its corresponding hash command type.
   Returns its list command type if conversion succeeds, NONE otherwise. */
static hash_cmd_type
convert_to_hash_cmd_type (const char *cmd)
{
  ASSERT (cmd != NULL);

  for (int i = 0; i < HASH_CMD_COUNT; ++i)
    if (strcmp (cmd, hash_cmd_table[i].name) == 0)
      return hash_cmd_table[i].type;

  printf ("%s: command not found\n", cmd);
  return NONE;
}

/* Computes and returns the hash value for hash element E, given
   auxiliary data AUX. */
static unsigned hash (const struct hash_elem *e, void *aux)
{
  ASSERT (e != NULL);

  struct hash_item *item = hash_entry (e, struct hash_item, elem);

  return hash_int (item->data);
}

/* Creates a new hash table with the name of ARGV[1]. */
static void
execute_create (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  if (find_hash_table_entry (argv[1]) != NULL)
    {
      printf ("%s: already exists\n", argv[1]);
      return;
    }

  if (is_hash_table_full ())
    {
      printf ("Hash table is full\n");
      return;
    }

  struct hash_table *new_entry = get_empty_hash_table_entry ();
  memcpy (new_entry->name, argv[1], strlen (argv[1]) + 1);
  if (hash_init (&new_entry->hash, hash, compare, NULL) == false)
    {
      printf ("Failed to create hash table.\n");
      memset (new_entry, '\0', sizeof (*new_entry));
    }
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

/* Finds a hash table entry that has the same name as ARG.
   Returns a pointer to the hash table entry if search succeeds,
   NULL otherwise. */
static struct hash_table *
find_hash_table_entry (const char *arg)
{
  ASSERT (arg != NULL);

  for (int i = 0; i < MAX_HASH_COUNT; ++i)
    if (strcmp (arg, hash_table[i].name) == 0)
      return &hash_table[i];

  return NULL;
}

/* Returns an empty hash table entry, NULL if hash table is full. */
static struct hash_table *
get_empty_hash_table_entry (void)
{
  for (int i = 0; i < MAX_HASH_COUNT; ++i)
      if (hash_table[i].name[0] == '\0')
        return &hash_table[i];

  return NULL;
}
