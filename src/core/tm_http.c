#include "timaios.h"

/**
 * set HTTP Header data.
 *
 * @param connection instance
 *
 */
void tm_http_write_header(tm_connection_t *_connection)
{
  struct tm *date;
  char *datestr;
  
  /* set request time */
  datestr = tm_memory_allocate((size_t)256);

  // tm_debug("_connection->request->timestamp : %d\n", (int)_connection->request->timestamp);

  date = localtime(&_connection->request->timestamp);
  strftime(datestr, 255, "%A, %d %B %Y %H:%M:%S GMT", date);
  
  sprintf(_connection->response->header,
    "HTTP/1.1 %d %s\r\n"
    "Cache-Control: private, max-age=0\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: %ld\r\n"
    "Server: timaios/%s\r\n"
    "Date: %s\r\n",
    _connection->response->status,
    tm_http_from_status_code_to_string(_connection->response->status),
    strlen(_connection->response->data),
    TM_VERSION,
    datestr
  );
  
  tm_memory_free(datestr);
}


/**
 * convert HTTP Status Code into string
 *
 * @param _status_code HTTP Status Code for the request
 *
 */
const char *tm_http_from_status_code_to_string(int _status_code)
{
  /* @TODO Implement other code */
  switch(_status_code) {
    case TM_HTTP_STATUS_OK:
      return "OK";
    case TM_HTTP_STATUS_MOVED_TEMPORARILY:
      return "Moved Temporarily";
    case TM_HTTP_STATUS_BAD_REQUEST:
      return "Bad Request";
    case TM_HTTP_STATUS_FORBIDDEN:
      return "Forbidden";
    case TM_HTTP_STATUS_NOT_FOUND:
      return "NOT FOUND";
    case TM_HTTP_STATUS_INTERNAL_SERVER_ERROR:
      return "Internal Server Error";
    case TM_HTTP_STATUS_SERVICE_UNAVAILABLE:
      return "Service Unavailable";
    default:
      return "UNKNOWN";
  }
}
