#include "timaios.h"

/**
 * Initialize configuration
 *
 */
TM_RETURN_STATUS tm_init_configuration(int _argc, char *_argv[])
{
  int ret;
  
  configuration.port = 12345;
  configuration.nevents = 32;
  configuration.pid_file = "./timaios.pid";
  configuration.is_daemon = 0;
  configuration.log_file = "./timaios.log";
  
  /* @TODO Validation? */
  while((ret = getopt(_argc, _argv,
    "h"   /* show version and usage(available options)  */
    "d"   /* start as daemon                            */
    "p:"  /* port number to wait requests               */
    "P:"  /* pid file path                              */
    "l:"  /* log file path                              */
    )) != -1){
    switch(ret){
      case 'h':
        tm_print_usage();
        return -1;
      case 'd':
        configuration.is_daemon = 1;
        break;
      case 'p':
        configuration.port = atoi(optarg);
        break;
      case 'P':
        configuration.pid_file = optarg;
        break;
      case 'l':
        configuration.log_file = optarg;
        break;
      default:
        tm_print_usage();
        return -1;
    }
  }
  
  return 0;
}


/**
 * Print usage.
 *
 */
void tm_print_usage()
{
  printf(
    "timaios %s\n"
    "\n"
    "-h         show version and usage(available options).\n"
    "-d         start as daemon.\n"
    "-p <num>   port number to wait request.\n"
    "-P <file>  file path for pid file.\n"
    ,TM_VERSION
  );
}
