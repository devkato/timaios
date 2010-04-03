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

  _connection->response->status = TM_HTTP_STATUS_OK;
}
