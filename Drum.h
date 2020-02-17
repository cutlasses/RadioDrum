#pragma once

#include <array>
#include "SamplePlayer.h"

////////////////////////////////////////////////////////////
// plays a single drum hit
class DRUM
{
  static constexpr int NUM_VOICES_PER_DRUM                                    = 2;
  
  std::array< SAMPLE_PLAYER_EFFECT, NUM_VOICES_PER_DRUM>    m_voices;
  POLYPHONIC_SAMPLE_PLAYER<NUM_VOICES_PER_DRUM>             m_poly_player;
  
public:

  DRUM( const uint16_t* sample_data );

  inline SAMPLE_PLAYER_EFFECT&                           voice( int vi )        { return m_voices[vi]; }
  static constexpr int                                   num_voices_per_drum()  { return NUM_VOICES_PER_DRUM; }
  static constexpr float                                 voice_mix()            { return (1.0f / NUM_VOICES_PER_DRUM); }

  void                                                   trigger( int pitch, float gain );
};

static constexpr int MAX_DRUMS                                                = 5;
using DRUM_SET = std::array<DRUM*, MAX_DRUMS>;

////////////////////////////////////////////////////////////
// a sequence for a single drum
class SEQUENCE
{
  struct TRIGGER
  {
    static constexpr int EMPTY                                                = -127;
    int8_t                                                m_pitch             = EMPTY;
    uint8_t                                               m_velocity          = 255;
  };
  
  static constexpr int MAX_SEQUENCE_SIZE                                      = 32;
  DRUM*                                                   m_drum              = nullptr;
  std::array<TRIGGER, MAX_SEQUENCE_SIZE>                  m_sequence;
  int8_t                                                  m_beat              = 0;
  int8_t                                                  m_sequence_length   = 0;
  
public:

  SEQUENCE()                                              {}
  SEQUENCE( DRUM& drum );

  int                                                     sequence_length() const;

  bool                                                    read(File& file);
  bool                                                    clock(int id);
};

using SEQUENCE_SET = std::array<SEQUENCE, MAX_DRUMS>;

////////////////////////////////////////////////////////////
// a PATTERN ties together each drum to each sequence
class PATTERN
{
  SEQUENCE_SET                                            m_sequences;
  uint8_t                                                 m_leading_sequence = 0; // the longest sequence, when this ends we can change the pattern
  
public:

  bool                                                    read( const char* filename, const DRUM_SET& drums ); 

  bool                                                    clock();   // returns true if this clock cycle ends the loop                          
};

////////////////////////////////////////////////////////////
// A set of patterns that can e cycle through
class PATTERN_SET
{
  static constexpr int MAX_PATTERNS                       = 4;
  std::array<PATTERN, MAX_PATTERNS>                       m_patterns;

  uint8_t                                                 m_num_patterns    = 0;
  uint8_t                                                 m_current_pattern = 0;
  uint8_t                                                 m_pending_pattern = 0;

public:

  bool                                                    is_pattern_pending() const;
  int                                                     current_pattern() const;
  int                                                     pending_pattern() const;

  void                                                    read( const DRUM_SET& drums );
  void                                                    advance_pending_pattern();  
  void                                                    clock();
};
