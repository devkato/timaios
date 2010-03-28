/**
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/ioctl.h>


/**
 *
 *
 */
#define TM_NEVENTS                32
#define TM_REQUEST_MAX_READ_SIZE  1024
#define TM_SERVER_PORT            12345
#define TM_PID_FILE_NAME          "timaios.pid"

typedef int TM_RETURN_STATUS;
typedef int TM_SERVER_SOCKET;

enum tm_connection_status {
  TM_CONNECTION_READ  =  1,
  TM_CONNECTION_WRITE =  2,
  TM_CONNECTION_ERROR = -1
};

#define TM_RETURN_STATUS_OK  0
#define tm_perror(msg) perror(msg)
#define tm_debug(...) (printf("tm_debug : "__VA_ARGS__))


/**
 *
 *
 */
#include "tm_memory.h"


/**
 *
 *
 */
#include "tm_epoll.h"


/**
 *
 *
 */
#include "tm_signal_handler.h"


/**
 *
 *
 */
#include "tm_pid.h"


/**
 *
 *
 */
#include "tm_socket.h"


/**
 *
 *
 */
#include "tm_http_request.h"


/**
 *
 *
 */
#include "tm_connection.h"
