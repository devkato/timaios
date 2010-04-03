#include "timaios.h"

/**
 * Open file and return file descriptor
 *
 * @param _filename
 * @param _flags
 * @param _mode
 *
 * @return file descriptor for the file
 */
int tm_open_file(const char *_filename, int _flags, int _mode)
{
  return open(_filename, _flags, _mode);
}


/**
 * initialize log file
 *
 */
int tm_logger_init()
{
  return tm_open_file(configuration.log_file, TM_FILE_OPEN_FLAG, TM_FILE_OPEN_MODE);
}


/**
 * write data into log file.
 *
 * @param _line data to be written
 */
void tm_logger_write(char *_line)
{
  struct iovec iovec[1];
  
  /* @TODO memory leak? */
  iovec[0].iov_base = _line;
  iovec[0].iov_len = strlen(_line);

  tm_writev(configuration.log_fd, iovec, 1);
}
