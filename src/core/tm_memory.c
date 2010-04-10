#include "timaios.h"

/**
 * reset memory with the pointer to zero
 *
 * @param _pointer pointer of the target struct
 * @param _size sizeof the struct
 */
void tm_memory_reset(void *_pointer, size_t _size)
{
  memset(_pointer, 0, _size);
}


/**
 * allocate memory
 *
 * @param _size byte size to allocate
 */
void *tm_memory_allocate(size_t _size)
{
  // if (_connection)
  //   pthread_mutex_lock(&_connection->mutex);
  
  void *p = malloc(_size);
  if (p == NULL) {
    tm_debug("can't allocate memory for size : %d", (int)_size);
  }
  
  // if (_connection)
  //   pthread_mutex_unlock(&_connection->mutex);

  return p;
}


/**
 * free memory
 *
 * @pointer _pointer of the memory to be free
 */
void tm_memory_free(void *_pointer)
{
  free(_pointer);
}
