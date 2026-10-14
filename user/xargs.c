#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define WORDLEN 512

static char *cmdargv[MAXARG + 1]; // +1 for the terminating 0
static char words[MAXARG][WORDLEN];
static int nbase;  // cmd + fixed args
static int nwords; // words collected for the current run
static int failed;

static char inbuf[512];
static int inpos, inlen;

// read one byte from stdin; returns -1 at EOF
static int
getbyte(void)
{
  if (inpos == inlen) {
    inlen = read(0, inbuf, sizeof(inbuf));
    inpos = 0;
    if (inlen <= 0) {
      inlen = 0;
      return -1;
    }
  }
  return (unsigned char)inbuf[inpos++];
}

static void
usage(void)
{
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

static void
run(void)
{
  int i, pid, status;

  if (nwords == 0)
    return;
  for (i = 0; i < nwords; i++)
    cmdargv[nbase + i] = words[i];
  cmdargv[nbase + nwords] = 0;

  pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    exec(cmdargv[0], cmdargv);
    fprintf(2, "xargs: exec %s failed\n", cmdargv[0]);
    exit(1);
  }
  status = 0;
  wait(&status);
  if (status != 0)
    failed = 1;
  nwords = 0;
}

static int
isspace(int c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int
main(int argc, char *argv[])
{
  int i, c, n = 0, first = 1, maxw, wlen = 0;
  char *s;

  if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
    if (argc < 4)
      usage();
    s = argv[2];
    if (*s == 0)
      usage();
    for (; *s; s++)
      if (*s < '0' || *s > '9')
        usage();
    n = atoi(argv[2]);
    if (n < 1)
      usage();
    first = 3;
  }
  if (first >= argc)
    usage();

  nbase = argc - first;
  if (nbase >= MAXARG) {
    fprintf(2, "xargs: too many arguments\n");
    exit(1);
  }
  for (i = 0; i < nbase; i++)
    cmdargv[i] = argv[first + i];

  // most words we can append per run
  maxw = MAXARG - nbase;
  if (n > 0 && n < maxw)
    maxw = n;

  for (;;) {
    c = getbyte();
    if (c == -1 || isspace(c)) {
      if (wlen > 0) { // finish current word
        words[nwords][wlen] = 0;
        nwords++;
        wlen = 0;
        if (nwords == maxw)
          run();
      }
      if (c == -1) {
        run(); // leftover words
        break;
      }
      if (c == '\n' && n == 0)
        run(); // no -n: one run per non-empty line
    } else {
      if (wlen < WORDLEN - 1)
        words[nwords][wlen++] = c;
    }
  }
  exit(failed ? 1 : 0);
}
