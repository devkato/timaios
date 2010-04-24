#include "timaios.h"

/**
 * copy string from one pinter to the other
 * 
 * @TODO Segmentation Fault occurs on multi-core environment.
 *
 * @param _dest destination pointer
 * @param _src source pointer
 */
void tm_strcpy(char *_dest, char *_src)
{
  if (_src == NULL) {
    return;
  }
  
  while ((*_dest++ = *_src++));
  // if (_src == NULL) {
  //   return;
  // }
  
  // while ((*_dest++ = *_src++));
  // while (*_dest != '\0') {
  //   *_dest = *_src;
  //   _dest++;
  //   _src++;
  // }
  return;
}


/**
 * copy string from one pinter to the other with limit
 *
 * @param _dest destination pointer
 * @param _src source pointer
 * @param _max max length to copy
 */
void tm_strcpyn(char *_dest, char *_src, int _max)
{
  int i = 0;
  while ((*_dest++ = *_src++)) {
    i++;
    if (i == _max) {
      return;
    }
  }
  return;
}
