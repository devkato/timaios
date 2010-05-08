#include "timaios.h"

/**
 * add action to action mapping object.
 *
 * @TODO should replace with Hash.
 * 
 * @param _index the index of function
 * @param _path the path to be associated with the function
 * @param _func the pointer of function to execute
 *
 */
void tm_action_add(int _index, const char *_path, void* _func)
{
  tm_action_t *action = tm_memory_allocate(sizeof(_path) + sizeof(*_func));
  action->url = (char *)_path;
  action->func = _func;
  action_map[_index] = action;
  
  tm_debug("add action[%d] for %s", _index, _path);
}


/**
 * find action associated with path
 * 
 * @param _path the path of request
 *
 * @return the pointer of registered function to process the request
 */
tm_action_t *tm_action_find(char *_path)
{
  /* @TODO make this operation with O(1) algorithm */
  int i;
  for (i = 0; i < 10; i++) {
    if (!action_map[i]) {
      tm_debug("no action found.");
      return NULL;
    }
    
    if (strcmp((action_map[i])->url, _path) == 0) {
      tm_debug("function for %s is selected.", _path);
      return action_map[i];
    }
  }
  
  tm_debug("no action found.");
  
  return NULL;
}


/**
 *
 * @TODO more better solution?
 *
 * @param _connection connection object
 *
 */
void tm_action_not_found(tm_connection_t *_connection)
{
  sprintf(_connection->response->data,
    "<html>\n"
    " <head>\n"
    "   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
    "   <title>Timaios - High Performance Dynamic HTTP Server</title>\n"
    " </head>\n"
    " <body>\n"
    " action associated with \"%s\" is not found.\n"
    " </body>\n"
    "</html>\n",
    _connection->request->path
  );
}
