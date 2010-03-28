#include "timaios.h"

/**
 *
 *
 *
 */
void tm_handle_signal_SIGINT()
{
  tm_debug("SIGINT sent.\n");
  tm_remove_pid_file(getpid(), TM_PID_FILE_NAME);
}
