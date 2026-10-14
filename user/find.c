#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define WANT_ALL  0
#define WANT_FILE 1
#define WANT_DIR  2

static int want = WANT_ALL;
static char *pattern;

// One shared path buffer. Each level appends "/name" and
// truncates back afterwards, so deep trees don't use much stack.
static char path[512];

// Does string s match pattern p? '*' matches any sequence (even empty).
static int
match(const char *p, const char *s)
{
  if (*p == 0)
    return *s == 0;
  if (*p == '*') {
    // let '*' match empty, or eat one char of s and try again
    return match(p + 1, s) || (*s != 0 && match(p, s + 1));
  }
  if (*s != 0 && *p == *s)
    return match(p + 1, s + 1);
  return 0;
}

static int
typeok(short type)
{
  if (want == WANT_FILE)
    return type == T_FILE || type == T_DEVICE;
  if (want == WANT_DIR)
    return type == T_DIR;
  return 1;
}

// path holds a directory; list it and recurse.
static void
find(void)
{
  int fd, len;
  struct dirent de;
  struct stat st;
  char name[DIRSIZ + 1];

  if ((fd = open(path, O_RDONLY)) < 0)
    return;
  if (fstat(fd, &st) < 0 || st.type != T_DIR) {
    close(fd);
    return;
  }

  len = strlen(path);
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    if (len + 1 + strlen(name) + 1 > sizeof(path)) {
      fprintf(2, "find: path too long\n");
      continue;
    }
    // append "/name" (avoid "//" if dir already ends with '/')
    char *q = path + len;
    if (len > 0 && path[len - 1] != '/')
      *q++ = '/';
    strcpy(q, name);

    if (stat(path, &st) >= 0) {
      if (match(pattern, name) && typeok(st.type))
        printf("%s\n", path);
      if (st.type == T_DIR)
        find();
    }
    path[len] = 0; // truncate back
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int fd;

  if (argc == 5 && strcmp(argv[3], "-type") == 0) {
    if (strcmp(argv[4], "f") == 0)
      want = WANT_FILE;
    else if (strcmp(argv[4], "d") == 0)
      want = WANT_DIR;
    else {
      fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
      exit(1);
    }
  } else if (argc != 3) {
    fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
    exit(1);
  }

  if (strlen(argv[1]) >= sizeof(path)) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  if ((fd = open(argv[1], O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
  }
  close(fd);

  pattern = argv[2];
  strcpy(path, argv[1]);
  find();
  exit(0);
}
