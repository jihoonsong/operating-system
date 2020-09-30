#include "list_handler.h"
#include <assert.h>	// Instead of 	#include "../debug.h"
#include <stdio.h>

#define ASSERT(CONDITION) assert(CONDITION)	// patched for proj0-2

/* TODO: Complete document. */
void
list_handler_invoke (const char *cmd, const int argc,
                     const char *argv[])
{
  printf ("list handler is invoked\n");
}
