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
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>

#define TM_VERSION                "0.0.4"

/**
 *
 *
 */
#define TM_REQUEST_MAX_READ_SIZE  1024
#define TM_SERVER_PORT            12345
#define TM_REQ_LINE_BREAK         "\r\n"
#define TM_REQ_LINE_BLANK         " "

#define TM_REQUEST_URL_MAX_LENGTH 1024
#define TM_HTTP_METHOD_MAX_LENGTH 8
#define TM_RESPONSE_DATA_MAX_LENGTH 8192

/* global configuration setting */
extern struct tm_configuration configuration;

typedef int TM_RETURN_STATUS;
typedef int TM_SERVER_SOCKET;


/**
 * HTTP Method Definition
 *
 */
enum tm_http_method {
  TM_HTTP_METHOD_GET  = 1,
  /* Currently not to support */
  TM_HTTP_METHOD_POST = 2
};


/**
 * HTTP Status Codes
 *
 */
enum tm_http_status_code {
  TM_HTTP_STATUS_OK                     = 200,
  TM_HTTP_STATUS_MOVED_TEMPORARILY      = 302,
  TM_HTTP_STATUS_BAD_REQUEST            = 400,
  TM_HTTP_STATUS_FORBIDDEN              = 403,
  TM_HTTP_STATUS_NOT_FOUND              = 404,
  TM_HTTP_STATUS_INTERNAL_SERVER_ERROR  = 500,
  TM_HTTP_STATUS_SERVICE_UNAVAILABLE    = 503,
  TM_HTTP_STATUS_UNKNOWN                = -1
};


/**
 * Return value of functions
 *
 */
enum tm_return_status {
  TM_RETURN_STATUS_SUCCESS  = 0,
  TM_RETURN_STATUS_ERROR    = -1
};


/**
 * Connection Status
 *
 */
enum tm_connection_status {
  TM_CONNECTION_READ  =  1,
  TM_CONNECTION_WRITE =  2,
  TM_CONNECTION_ERROR = -1
};


/**
 * printing message functions
 *
 */
#define tm_perror(msg) perror(msg)

# ifdef TM_PRODUCTION

#define tm_debug(...) 

# else

#define tm_debug(...) (printf("[DEBUG] "__VA_ARGS__))

# endif


/**
 *
 *
 */
#include "tm_config.h"


/**
 *
 *
 */
#include "tm_memory.h"


/**
 *
 *
 */
#include "tm_string.h"


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
#include "tm_http.h"


/**
 *
 *
 */
#include "tm_connection.h"

void tm_http_write_header(tm_connection_t *_connection);
typedef struct tm_action {
  char *url;
  void (*func)(tm_connection_t *_connection);
} tm_action_t;

extern tm_action_t* action_map[10];


/**
 *
 *
 */
#include "tm_socket.h"


/**
 *
 *
 */
#include "tm_logger.h"


/**
 *
 *
 */
#include "tm_time.h"


/**
 *
 *
 */
#include "tm_action.h"


/**
 * Action definitions
 *
 */
#include "../action/tm_action_definition.h"

