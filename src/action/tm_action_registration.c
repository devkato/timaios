#include "../core/timaios.h"

/**
 * register action
 *
 * @TODO more better and smart solution?
 *
 */
void tm_acton_regiser_init()
{  
  tm_action_add(0, "/", tm_action_root);
  tm_action_add(1, "/echo", tm_action_echo);
}
