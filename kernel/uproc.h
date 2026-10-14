struct uproc {
  int pid;
  int ppid;       // parent's pid, or 0 if there is no parent
  int state;      // enum procstate value from kernel/proc.h
  uint64 sz;      // size of user memory in bytes
  char name[16];
};
