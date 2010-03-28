
/**
 *
 *
 *
 */
typedef struct tm_connection {
  int fd;
  char raw_data[TM_REQUEST_MAX_READ_SIZE];
  tm_http_request_t *request;
  int status;
  int n;
} tm_connection_t;
