#include "hash_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

/* TODO: Complete document. */
void
hash_handler_invoke (const char *cmd, const int argc,
                     const char *argv[])
{
  printf ("hash handler is invoked\n");
}
