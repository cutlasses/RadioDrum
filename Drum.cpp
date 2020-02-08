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
  // default sequence
  m_sequence_size = 8;
//  m_sequence      = { {0,255}, {0,255}, {0,255}, {0,255}, {0,255}, {0,255}, {0,255}, {0,255} };
}

bool SEQUENCE::read(File& file)
{
  return false;
}

void SEQUENCE::clock()
{
  const TRIGGER& trig = m_sequence[m_beat];
  if( trig.m_pitch >= 0 )
  {
    // TODO use value to set pitch
    m_drum.trigger();
  }

  m_beat = (m_beat + 1) % m_sequence_size;
}
