#pragma once

#include <array>
#include "SamplePlayer.h"

class DRUM
{
  static constexpr int NUM_VOICES_PER_DRUM                  = 2;
  
  std::array< SAMPLE_PLAYER_EFFECT, NUM_VOICES_PER_DRUM>    m_voices;
  POLYPHONIC_SAMPLE_PLAYER<NUM_VOICES_PER_DRUM>             m_poly_player;
  
public:

  DRUM( const uint16_t* sample_data );

  inline SAMPLE_PLAYER_EFFECT&                           voice( int vi )      { return m_voices[vi]; }
  static constexpr float                                 voice_mix()          { return 1.0f / NUM_VOICES_PER_DRUM; }
};
