#include "Drum.h"

////////////////////////////////////////////////////////////

DRUM::DRUM( const uint16_t* sample_data ) :
  m_voices(),
  m_poly_player(sample_data)
{
  for( SAMPLE_PLAYER_EFFECT& voice : m_voices )
  {
    m_poly_player.add_sample_player( voice );
  }
}

void DRUM::trigger()
{
  m_poly_player.play(1.0f);
}

////////////////////////////////////////////////////////////

SEQUENCE::SEQUENCE( DRUM& drum ) :
  m_drum(drum)
{
  
}

void SEQUENCE::clock()
{
  const int8_t trig = m_sequence[m_beat];
  if( trig >= 0 )
  {
    // TODO use value to set pitch
    m_drum.trigger();
  }

  m_beat = (m_beat + 1) % SEQUENCE_SIZE;
}
