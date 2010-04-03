#include "../core/timaios.h"


/**
 *
 * @TODO To plugin
 * @TODO HTTP Status code implementation
 * @TODO content-type implementation to tm_http_request
 * @TODO Content-Length implementation
 *
 */
void tm_action_root(tm_connection_t *_connection)
{
  time_t timer;
  struct tm *date;
  char *datestr = tm_memory_allocate((size_t)256);

  /* response body */
  sprintf(_connection->response->data,
    "<html>\n"
    " <head>\n"
    "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
    "   <title>Timaios - High Performance Dynamic HTTP Server</title>\n"
    " </head>\n"
    " <body>\n"
    " Project site on GitHub is <a href=\"http://github.com/devkato/timaios\">here</a>\n"
    " </body>\n"
    "</html>\n"
  );

  /* response header */
  timer = time(NULL);
  date = localtime(&timer);
  strftime(datestr, 255, "%A, %d %B %Y %H:%M:%S GMT", date);
  
  sprintf(_connection->response->header,
    "HTTP/1.1 200 OK\r\n"
    "Cache-Control: private, max-age=0\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: %ld\r\n"
    "Server: timaios/%s\r\n"
    "Date: %s\r\n",
    strlen(_connection->response->data),
    TM_VERSION,
    datestr
  );
  
  tm_memory_free(datestr);
}
