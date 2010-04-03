#include "timaios.h"

#define TM_FILE_OPEN_FLAG O_CREAT | O_APPEND | O_WRONLY | O_NONBLOCK | O_ASYNC
#define TM_FILE_OPEN_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

int tm_open_file(const char *filename, int flags, int mode)
{
  return open(filename, flags);
}


/**
 * Open file and return file descriptor
 *
 *
 */
int tm_logger_init()
{
  return tm_open_file(configuration.log_file, TM_FILE_OPEN_FLAG, TM_FILE_OPEN_MODE);
}

void tm_logger_write(char *line)
{
  struct iovec iovec[1];
  
  /* @TODO memory leak? */
  iovec[0].iov_base = line;
  iovec[0].iov_len = strlen(line);

  tm_writev(configuration.log_fd, iovec, 1);
}
