#include "Drum.h"

DRUM::DRUM( const uint16_t* sample_data ) :
  m_voices(),
  m_poly_player(sample_data)
{
  for( SAMPLE_PLAYER_EFFECT& voice : m_voices )
  {
    m_poly_player.add_sample_player( voice );
  }
}
