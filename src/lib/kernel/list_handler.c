#include "list_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2
#define DECIMAL 10
#define MAX_LIST_COUNT 10
#define MAX_LIST_NAME 100

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

/* Execution functions. */
static bool compare (const struct list_elem *a, const struct list_elem *b,
                     void *aux);
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

/* List table functions. */
static struct list_table *find_list_table_entry (const char *arg);
static struct list_table *get_empty_list_table_entry (void);

/* List item functions. */
static void delete_list_item (struct list_item *item);
static struct list_elem *find_list_element_at (struct list *list,
                                               const int index);
static struct list_item *new_list_item (const char *arg);

/* Miscellaneous functions. */
static int convert_to_index (const char *arg);

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

/* List table. */
struct list_table
  {
    char name[MAX_LIST_NAME];
    struct list list;
  };
static struct list_table list_table[MAX_LIST_COUNT];

/* Returns true if list table is full, false otherwise. */
static inline bool
is_list_table_full (void)
{
  for (int i = 0; i < MAX_LIST_COUNT; ++i)
    if (list_table[i].name[0] == '\0')
      return false;

  return true;
}

/* Initializes list table. */
void
list_handler_initialize (void)
{
  srand ((unsigned int) time (NULL)); // list_shuffle() needs this.

  memset (list_table, '\0', sizeof (list_table));
}

/* Releases memory. */
void
list_handler_terminate (void)
{
  for (int i = 0; i < MAX_LIST_COUNT; ++i)
    if (list_table[i].name[0] != '\0')
      while (!list_empty (&list_table[i].list))
        {
          struct list_elem *element = list_pop_front (&list_table[i].list);
          struct list_item *item = list_entry (element, struct list_item, elem);
          delete_list_item (item);
        }
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

/* Compares the value of two list elements A and B, given
   auxiliary data AUX.  Returns true if A is less than B, or
   false if A is greater than or equal to B. */
static bool
compare (const struct list_elem *a, const struct list_elem *b, void *aux)
{
  ASSERT (a != NULL);
  ASSERT (b != NULL);

  struct list_item *item_a = list_entry (a, struct list_item, elem);
  struct list_item *item_b = list_entry (b, struct list_item, elem);

  return item_a->data < item_b->data ? true : false;
}

/* Converts CMD to its corresponding list command type.
   Returns its list command type if conversion succeeds, NONE otherwise. */
static list_cmd_type
convert_to_list_cmd_type (const char *cmd)
{
  ASSERT (cmd != NULL);

  for (int i = 0; i < LIST_CMD_COUNT; ++i)
    if (strcmp (cmd, list_cmd_table[i].name) == 0)
      return list_cmd_table[i].type;

  printf ("%s: command not found\n", cmd);
  return NONE;
}

/* Creates a new list with the name of ARGV[1]. */
static void
execute_create (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  if (find_list_table_entry (argv[1]) != NULL)
    {
      printf ("%s: already exists\n", argv[1]);
      return;
    }

  if (is_list_table_full ())
    {
      printf ("List table is full\n");
      return;
    }

  struct list_table *new_entry = get_empty_list_table_entry ();
  memcpy (new_entry->name, argv[1], strlen (argv[1]) + 1);
  list_init (&new_entry->list);
}

/* Deletes a list with the name of ARGV[1]. */
static void
execute_delete (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    return;

 while (!list_empty (&entry->list))
   {
     struct list_elem *element = list_pop_front (&entry->list);
     struct list_item *item = list_entry (element, struct list_item, elem);
     delete_list_item (item);
   }
  memset (entry, '\0', sizeof (*entry));
}

/* Prints all data stored in a list with a name of ARGV[0]. */
static void
execute_dumpdata (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    return;

  if (list_empty (&entry->list))
    return;

  for (struct list_elem *element = list_begin (&entry->list);
       element != list_end (&entry->list); element = list_next (element))
    {
      struct list_item *item = list_entry (element, struct list_item, elem);
      printf ("%d ", item->data);
    }
  printf ("\n");
}

/* Prints the data stored in the list item at the end of the list with the
   same name as ARGV[0]. Undefined behavior if the list is empty. */
static void
execute_list_back (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *item = list_entry (list_back (&entry->list),
                                       struct list_item, elem);
  printf ("%d\n", item->data);
}

/* Prints true if the list with the same name as ARGV[0] is empty,
   false otherwise. */
static void
execute_list_empty (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  printf (list_empty (&entry->list) ? "true\n" : "false\n");
}

/* Prints the data stored in the list item at the beginning of the list with
   the same name as ARGV[0]. Undefined behavior if the list is empty. */
static void
execute_list_front (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *item = list_entry (list_front (&entry->list),
                                       struct list_item, elem);
  printf ("%d\n", item->data);
}

/* Inserts a new list item with the data of ARGV[2] at the ARGV[1]-th position
   at a list with the name of ARGV[0]. If ARGV[1] is bigger than the size of
   the list, a new list item is inserted at the end of the list. */
static void
execute_list_insert (const int argc, const char *argv[])
{
  ASSERT (argc == 3);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (argv[2] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  int index = convert_to_index (argv[1]);
  if (index < 0)
    return;

  struct list_item *new_item = new_list_item (argv[2]);
  if (new_item == NULL)
    return;

  list_insert (find_list_element_at (&entry->list, index), &new_item->elem);
}

/* Inserts a new list item with the data of ARGV[1] in the proper position
   in a list with the name of ARGV[0], which must be sorted in nondecreasing
   order. */
static void
execute_list_insert_ordered (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *new_item = new_list_item (argv[1]);
  if (new_item == NULL)
    return;

  list_insert_ordered (&entry->list, &new_item->elem, compare, NULL);
}

/* Prints the biggest data in a list with the name of ARGV[0]. If there is
   more than one maximum, returns the one that appears earlier in the list.
   If the the list is empty, prints nothing. */
static void
execute_list_max (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_elem *max = list_max (&entry->list, compare, NULL);
  if (max == list_end (&entry->list))
    return;

  struct list_item *item = list_entry (max, struct list_item, elem);
  printf ("%d\n", item->data);
}

/* Prints the smallest data in a list with the name of ARGV[0]. If there is
   more than one minimum, returns the one that appears earlier in the list.
   If the the list is empty, prints nothing. */
static void
execute_list_min (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_elem *min = list_min (&entry->list, compare, NULL);
  if (min == list_end (&entry->list))
    return;

  struct list_item *item = list_entry (min, struct list_item, elem);
  printf ("%d\n", item->data);
}

/* Removes the back element from the list with a name of ARGV[0] and prints
   its data. Undefined behavior if LIST is empty before removal. */
static void
execute_list_pop_back (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *item = list_entry (list_pop_back (&entry->list),
                                       struct list_item, elem);
  delete_list_item (item);
}

/* Removes the front element from the list with a name of ARGV[0] and prints
   its data. Undefined behavior if LIST is empty before removal. */
static void
execute_list_pop_front (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *item = list_entry (list_pop_front (&entry->list),
                                       struct list_item, elem);
  delete_list_item (item);
}

/* Creates a new list item with the data of ARGV[1] and inserts the item
   at the end of a list with the name of ARGV[0]. */
static void
execute_list_push_back (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *new_item = new_list_item (argv[1]);
  if (new_item == NULL)
    return;

  list_push_back (&entry->list, &new_item->elem);
}

/* Creates a new list item with the data of ARGV[1] and inserts the item
   at the beginning of a list with the name of ARGV[0]. */
static void
execute_list_push_front (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_item *new_item = new_list_item (argv[1]);
  if (new_item == NULL)
    return;

  list_push_front (&entry->list, &new_item->elem);
}

/* Removes the ARGV[1]-th list item at the list with a name of ARGV[0]. */
static void
execute_list_remove (const int argc, const char *argv[])
{
  ASSERT (argc == 2);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  int index = convert_to_index (argv[1]);
  if (index < 0)
    return;

  list_remove (find_list_element_at (&entry->list, index));
}

/* Reverses the order of a list with the name of ARGV[0]. */
static void
execute_list_reverse (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  list_reverse (&entry->list);
}

/* Shuffles a list with the name of ARGV[0]. */
static void
execute_list_shuffle (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  list_shuffle (&entry->list);
}

/* Returns the number of elements in a list with the name of ARGV[0].
   Runs in O(n) in the number of elements. */
static void
execute_list_size (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  printf ("%zu\n", list_size (&entry->list));
}

/* Sorts a list with the name of ARGV[0] in nondecreasing order. */
static void
execute_list_sort (const int argc, const char *argv[])
{
  ASSERT (argc == 1);
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  list_sort (&entry->list, compare, NULL);
}

/* Removes list elements in a list with a name of ARGV[2]
   from ARGV[3] to ARGV[4] (exclusive), then inserts them
   at the ARGV[1]-th position in a list with a name of ARGV[0]. */
static void
execute_list_splice (const int argc, const char *argv[])
{
  ASSERT (argc == 5);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (argv[2] != NULL);
  ASSERT (argv[3] != NULL);
  ASSERT (argv[4] != NULL);

  struct list_table *to_entry = find_list_table_entry (argv[0]);
  if (to_entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list_table *from_entry = find_list_table_entry (argv[2]);
  if (from_entry == NULL)
    {
      printf ("%s: list not found\n", argv[2]);
      return;
    }

  int insert_to = convert_to_index (argv[1]);
  if (insert_to < 0)
    return;

  int splice_start = convert_to_index (argv[3]);
  if (splice_start < 0)
    return;

  int splice_end = convert_to_index (argv[4]);
  if (splice_end < 0)
    return;

  list_splice (find_list_element_at (&to_entry->list, insert_to),
               find_list_element_at (&from_entry->list, splice_start),
               find_list_element_at (&from_entry->list, splice_end));
}

/* Swaps list elements at the ARGV[1]-th position and the ARGV[2]-th position
   in a list with the name of ARGV[0]. */
static void
execute_list_swap (const int argc, const char *argv[])
{
  ASSERT (argc == 3);
  ASSERT (argv[0] != NULL);
  ASSERT (argv[1] != NULL);
  ASSERT (argv[2] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  int a = convert_to_index (argv[1]);
  if (a < 0)
    return;

  int b = convert_to_index (argv[2]);
  if (b < 0)
    return;

  list_swap (find_list_element_at (&entry->list, a),
             find_list_element_at (&entry->list, b));
}

/* Iterates through a list with the name of ARGV[0] and removes all but
   the first in each set of adjacent elements that are equal according to
   COMPARE function. If ARGV[1] is non-null, then the removed elements are
   appended to a list with the name of ARGV[1]. */
static void
execute_list_unique (const int argc, const char *argv[])
{
  ASSERT (argc == 1 || (argc == 2 && argv[1] != NULL));
  ASSERT (argv[0] != NULL);

  struct list_table *entry = find_list_table_entry (argv[0]);
  if (entry == NULL)
    {
      printf ("%s: list not found\n", argv[0]);
      return;
    }

  struct list *duplicates = NULL;
  if (argv[1] != NULL)
    {
      struct list_table *_entry = find_list_table_entry (argv[1]);
      if (_entry == NULL)
        {
          printf ("%s: list not found\n", argv[1]);
          return;
        }

      duplicates = &_entry->list;
    }

  list_unique (&entry->list, duplicates, compare, NULL);
}

/* Finds a list table entry that has the same name as ARG.
   Returns a pointer to the list table entry if search succeeds,
   NULL otherwise. */
static struct list_table *
find_list_table_entry (const char *arg)
{
  ASSERT (arg != NULL);

  for (int i = 0; i < MAX_LIST_COUNT; ++i)
    if (strcmp (arg, list_table[i].name) == 0)
      return &list_table[i];

  return NULL;
}

/* Returns an empty list table entry, NULL if list table is full. */
static struct list_table *
get_empty_list_table_entry (void)
{
  for (int i = 0; i < MAX_LIST_COUNT; ++i)
      if (list_table[i].name[0] == '\0')
        return &list_table[i];

  return NULL;
}

/* Releases memory of ITEM. */
static void
delete_list_item (struct list_item *item)
{
  ASSERT (item != NULL);

  free (item);
}

/* Returns a list element at the INDEX-th position in LIST.
   If INDEX == list_size (LIST), then LIST->tail will be returned. */
static struct list_elem *
find_list_element_at (struct list *list, const int index)
{
  ASSERT (list != NULL);
  ASSERT (index >= 0 && index <= list_size (list));

  struct list_elem *element = list_begin (list);
  for (int i = 0; i < index && element != list_end (list); ++i)
      element = list_next (element);

  return element;
}

/* Allocates memory of list item with the data of ARG, and returns it. */
static struct list_item *
new_list_item (const char *arg)
{
  ASSERT (arg != NULL);

  char *endptr = NULL;
  int data = strtol (arg, &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", arg);
      return NULL;
    }

  struct list_item *new_item = malloc (sizeof (*new_item));
  new_item->elem.prev = NULL;
  new_item->elem.next = NULL;
  new_item->data = data;

  return new_item;
}

/* Converts ARG from an array of char to a nonnegative decimal.
   Returns -1 when ARG is not a decimal string. */
static int
convert_to_index (const char *arg)
{
  ASSERT (arg != NULL);

  char *endptr = NULL;
  int index = strtol (arg, &endptr, DECIMAL);
  if (*endptr != '\0')
    {
      printf ("%s: not decimal\n", arg);
      return -1;
    }
  if (index < 0)
    {
      printf ("%s: index cannot be negative\n", arg);
      return -1;
    }

  return index;
}
