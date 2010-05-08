#include "timaios.h"

/**
 * return (logical) CPU numbers.
 *
 */
int tm_sys_get_cpu_num()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
}


/**
 * return page-size of memory for this machine.
 *
 */
int tm_sys_getpagesize()
{
  return getpagesize();
}


/**
 * Detect maximum number of file descriptors.
 *
 */
int tm_get_max_fd_num()
{
  struct rlimit  rl;
  
  if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
    return -1;
  }
  
  return rl.rlim_cur;
}

