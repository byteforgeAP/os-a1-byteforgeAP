#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// returns 1 if s is a non-empty string of digits only
static int
alldigits(const char *s)
{
  if (*s == 0)
    return 0;
  for (; *s; s++)
    if (*s < '0' || *s > '9')
      return 0;
  return 1;
}

int
main(int argc, char *argv[])
{
  // exactly one argument, and it must be all digits
  // (this also rejects negatives, since '-' is not a digit)
  if (argc != 2 || !alldigits(argv[1])) {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  pause(atoi(argv[1]));
  exit(0);
}
