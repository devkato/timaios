#include "timaios.h"

/**
 *
 *
 *
 */
void tm_create_pid_file(pid_t _pid, const char *_pid_filename)
{
  FILE *fp;
  
  if (_pid_filename == NULL) {
    return;
  }
  
  if ((fp = fopen(_pid_filename, "w")) == NULL) {
    tm_perror("fopen");
    return;
  }
  
  fprintf(fp, "%ld\n", (long)_pid);
  
  if (fclose(fp) < 0) {
    tm_perror("fclose");
    return;
  }
}


/**
 *
 *
 *
 */
void tm_remove_pid_file(pid_t _pid, const char *_pid_filename)
{
  if (remove(_pid_filename) != 0) {
    tm_perror("remove");
    return;
  }
}
