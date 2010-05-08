#include "../core/timaios.h"

void tm_action_echo(tm_connection_t *_connection)
{
  // tm_debug("_connection->request->raw_data : \n%s\n", _connection->request->raw_data);
  
  /* response body */
  snprintf(_connection->response->data,
    TM_RESPONSE_DATA_MAX_LENGTH,
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
