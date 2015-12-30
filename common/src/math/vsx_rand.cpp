#include "math/vsx_rand.h"
#include <common/third-party/mersenne-twister/mersenne-twister.h>


vsx_rand::vsx_rand()
{
  state = new mt_state;
  srand(1);
}


~vsx_rand()
{
  delete state;
}

void srand(uint32_t seed)
{
  mts_seed32( state, seed );
}

uint32_t rand()
{
  return mts_lrand( state );
}

float frand()
{
  return (float)mts_drand( state );
}
