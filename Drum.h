#pragma once

#include <array>
#include "SamplePlayer.h"

////////////////////////////////////////////////////////////

class DRUM
{
  static constexpr int NUM_VOICES_PER_DRUM                                    = 2;
  
  std::array< SAMPLE_PLAYER_EFFECT, NUM_VOICES_PER_DRUM>    m_voices;
  POLYPHONIC_SAMPLE_PLAYER<NUM_VOICES_PER_DRUM>             m_poly_player;
  
public:

  DRUM( const uint16_t* sample_data );

  inline SAMPLE_PLAYER_EFFECT&                           voice( int vi )        { return m_voices[vi]; }
  static constexpr int                                   num_voices_per_drum()  { return NUM_VOICES_PER_DRUM; }
  static constexpr float                                 voice_mix()            { return 1.0f / NUM_VOICES_PER_DRUM; }

  void                                                   trigger();
};

static constexpr int MAX_DRUMS                                                = 5;
using DRUM_SET = std::array<DRUM*, MAX_DRUMS>;

////////////////////////////////////////////////////////////

class SEQUENCE
{
  struct TRIGGER
  {
    static constexpr int EMPTY                                                = -1;
    int8_t                                                m_pitch             = EMPTY;
    uint8_t                                               m_velocity          = 255;
  };
  
  static constexpr int MAX_SEQUENCE_SIZE                                      = 16;
  DRUM*                                                   m_drum              = nullptr;
  std::array<TRIGGER, MAX_SEQUENCE_SIZE>                  m_sequence;
  int8_t                                                  m_beat              = 0;
  int8_t                                                  m_sequence_size     = 0;
  
public:

  SEQUENCE()                                              {}
  SEQUENCE( DRUM& drum );

  bool                                                    read(File& file);
  void                                                    clock();
};

using SEQUENCE_SET = std::array<SEQUENCE, MAX_DRUMS>;

////////////////////////////////////////////////////////////

// a PATTERN ties together each drum to each sequence
class PATTERN
{
  SEQUENCE_SET                                            m_sequences;
  
public:

  void                                                    read( const char* filename, const DRUM_SET& drums ); 

  void                                                    clock();                             
};
