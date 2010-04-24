/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_CONFIG_
#define _TIMAIOS_HEADER_CONFIG_


typedef struct tm_configuration {
  char *pid_file;
  int nevents;
  int port;
  int is_daemon;
  char *log_file;
  int log_fd;
} tm_configuration_t;

typedef struct tm_action_map {
  
} tm_action_map_t;

void tm_print_usage();

TM_RETURN_STATUS tm_init_configuration(int argc, char *argv[]);


#endif
