#pragma once

#include <array>
#include "SamplePlayer.h"

////////////////////////////////////////////////////////////

class DRUM
{
  static constexpr int NUM_VOICES_PER_DRUM                  = 2;
  
  std::array< SAMPLE_PLAYER_EFFECT, NUM_VOICES_PER_DRUM>    m_voices;
  POLYPHONIC_SAMPLE_PLAYER<NUM_VOICES_PER_DRUM>             m_poly_player;
  
public:

  DRUM( const uint16_t* sample_data );

  inline SAMPLE_PLAYER_EFFECT&                           voice( int vi )      { return m_voices[vi]; }
  static constexpr float                                 voice_mix()          { return 1.0f / NUM_VOICES_PER_DRUM; }

  void                                                   trigger();
};

////////////////////////////////////////////////////////////

class SEQUENCE
{
  struct TRIGGER
  {
    int8_t                                                m_pitch             = -1;
    uint8_t                                               m_velocity          = 255;
  };
  
  static constexpr int MAX_SEQUENCE_SIZE                  = 16;
  DRUM&                                                   m_drum;
  std::array<TRIGGER, MAX_SEQUENCE_SIZE>                  m_sequence;
  int8_t                                                  m_beat              = 0;
  int8_t                                                  m_sequence_size     = 0;

  bool                                                    read(File& file);
  
public:

  SEQUENCE( DRUM& drum );
  void                                                    clock();
};
