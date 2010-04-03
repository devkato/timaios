#include "timaios.h"

/**
 * handle SIGINT signal
 *
 */
void tm_handle_signal_SIGINT()
{
  tm_debug("SIGINT sent.\n");
  tm_remove_pid_file(getpid(), configuration.pid_file);
}


