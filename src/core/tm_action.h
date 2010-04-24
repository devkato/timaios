/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_ACTION_
#define _TIMAIOS_HEADER_ACTION_


void tm_action_add(int _index, const char *_path, void* _func);

tm_action_t *tm_action_find(char *path);

void tm_action_not_found(tm_connection_t *_connection);


#endif
