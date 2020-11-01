#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  if (argc != 5)
    return EXIT_FAILURE;

  int num[4] = {atoi (argv[1]), atoi (argv[2]), atoi (argv[3]), atoi (argv[4])};
  int fib_ans = num[0];
  int max_ans = num[0];
  int fib = fibonacci (num[0]);
  int max = max_of_four_int (num[0], num[1], num[2], num[3]);

  int temp[2] = {0, 1};
  for (int i = 2; i <= num[0]; ++i)
    {
      fib_ans = temp[0] + temp[1];
      temp[0] = temp[1];
      temp[1] = fib_ans;
    }

  for (int i = 1; i < 4; ++i)
    if (max_ans < num[i])
      max_ans = num[i];

  printf ("%d %d\n", fib, max);

  return (fib == fib_ans) && (max == max_ans) ? EXIT_SUCCESS : EXIT_FAILURE;
}
