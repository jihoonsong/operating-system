#include "hash_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define DECIMAL 10
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
static hash_cmd_type convert_to_hash_cmd_type (const char *cmd);
static unsigned hash (const struct hash_elem *e, void *aux);
static void square (struct hash_elem *e, void *aux);
static void triple (struct hash_elem *e, void *aux);
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

/* Hash table functions. */
static struct hash_table *find_hash_table_entry (const char *arg);
static struct hash_table *get_empty_hash_table_entry (void);

/* Hash item functions. */
static void delete_hash_item (struct hash_elem *e, void *aux);
static struct hash_item *new_hash_item (const char *arg);

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

/* Replace data of E with squred data. */
static void
square (struct hash_elem *e, void *aux)
{
  ASSERT (e != NULL);

  struct hash_item *item = hash_entry (e, struct hash_item, elem);
  item->data = item->data * item->data;
}

/* Replace data of E with cubed data. */
static void
triple (struct hash_elem *e, void *aux)
{
  ASSERT (e != NULL);

  struct hash_item *item = hash_entry (e, struct hash_item, elem);
  item->data = item->data * item->data * item->data;
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

/* Deletes a hash table with the name of ARGV[1]. */
static void
execute_delete (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
      return;

  hash_destroy (&entry->hash, delete_hash_item);

  memset (entry, '\0', sizeof (*entry));
}

/* Prints all data stored in a hash table with a name of ARGV[0]. */
static void
execute_dumpdata (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
    return;

  if (hash_empty (&entry->hash))
    return;

  struct hash_iterator i;

  hash_first (&i, &entry->hash);
  while (hash_next (&i))
    {
      struct hash_item *item = hash_entry (hash_cur (&i),
                                           struct hash_item, elem);
      printf ("%d ", item->data);
    }
  printf ("\n");
}

/* Calls a function specified by ARGV[1] for each element in hash table with
   the name of ARGV[0]. Available functions are SQUARE and TRIPLE. */
static void
execute_hash_apply (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: hashtable not found\n", argv[0]);
      return;
    }

  if (strcmp(argv[1], "square") == 0)
    {
      hash_apply (&entry->hash, square);
      return;
    }

  if (strcmp(argv[1], "triple") == 0)
      hash_apply (&entry->hash, triple);
}

/* Clears a hash table with the name of ARGV[1]. */
static void
execute_hash_clear (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
      return;

  hash_clear (&entry->hash, delete_hash_item);
}

/* TODO: Complete document. */
static void
execute_hash_delete (const int argc, const char *argv[])
{
  printf ("execute_hash_delete\n");
}

/* Returns true if a hash table with the name of ARGV[0]
   contains no elements, false otherwise. */
static void
execute_hash_empty (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: hashtable not found\n", argv[0]);
      return;
    }

  printf (hash_empty (&entry->hash) ? "true\n" : "false\n");
}

/* TODO: Complete document. */
static void
execute_hash_find (const int argc, const char *argv[])
{
  printf ("execute_hash_find\n");
}

/* Inserts a new hash item with the data of ARGV[1] in a hash table with the
   name of ARGV[0], if a hash value of the item is not duplicate. */
static void
execute_hash_insert (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: hashtable not found\n", argv[0]);
      return;
    }

  struct hash_item *new_item = new_hash_item (argv[1]);
  if (new_item == NULL)
    return;

  if (hash_insert (&entry->hash, &new_item->elem) != NULL)
    delete_hash_item (&new_item->elem, NULL);
}

/* TODO: Complete document. */
static void
execute_hash_replace (const int argc, const char *argv[])
{
  printf ("execute_hash_replace\n");
}

/* Returns the number of elements in a hash table with the name of ARGV[0]. */
static void
execute_hash_size (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct hash_table *entry = find_hash_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: hashtable not found\n", argv[0]);
      return;
    }

  printf ("%zu\n", hash_size (&entry->hash));
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

/* Releases memory of ITEM. */
static void
delete_hash_item (struct hash_elem *e, void *aux)
{
  ASSERT (e != NULL);

  free (hash_entry (e, struct hash_item, elem));
}

/* Allocates memory of list hash with the data of ARG, and returns it. */
static struct hash_item *
new_hash_item (const char *arg)
{
  ASSERT (arg != NULL);

  char *endptr = NULL;
  int data = strtol (arg, &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", arg);
      return NULL;
    }

  struct hash_item *new_item = malloc (sizeof (*new_item));
  new_item->elem.list_elem.prev = NULL;
  new_item->elem.list_elem.next = NULL;
  new_item->data = data;

  return new_item;
}
