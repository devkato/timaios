#include "../core/timaios.h"

void tm_action_echo(tm_connection_t *_connection)
{
  /* response body */
  sprintf(_connection->response->data,
    "<html>\n"
    " <head>\n"
    "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
    "   <title>Timaios - High Performance Dynamic HTTP Server</title>\n"
    " </head>\n"
    " <body>\n"
    " Echo action called.\n"
    " %s"
    " </body>\n"
    "</html>\n",
    _connection->raw_data
  );

  _connection->response->status = TM_HTTP_STATUS_OK;
}
