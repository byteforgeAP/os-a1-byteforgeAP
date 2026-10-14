#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

// indexed by enum procstate: UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE
static char *states[] = {
  "unused", "used", "sleeping", "runnable", "running", "zombie",
};

// global, not on the (one-page) user stack
static struct uproc procs[NPROC];

int
main(void)
{
  int i, n;
  char *st;

  n = getprocs(procs, NPROC);
  if (n < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }
  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");
  for (i = 0; i < n; i++) {
    if (procs[i].state >= 0 && procs[i].state < 6)
      st = states[procs[i].state];
    else
      st = "???";
    printf("%d\t%d\t%s\t%lu\t%s\n", procs[i].pid, procs[i].ppid, st,
           procs[i].sz, procs[i].name);
  }
  exit(0);
}
