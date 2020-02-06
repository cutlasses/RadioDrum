#pragma once

#include <Audio.h>
#include "FixedPoint.h"
#include "Util.h"

/////////////////////////////////////////////////////////

constexpr int           SEMI_TONE_RANGE( 3.3f * 12 );

class SAMPLE_PLAYER_EFFECT : public AudioStream
{
  audio_block_t*        m_input_queue_array[1];

  const uint16_t*       m_sample_data;
  int                   m_sample_length;

  FIXED_POINT           m_speed;
  FIXED_POINT           m_read_head;

  //int16_t               read_sample_linear() const;
  int16_t               read_sample_linear_fp() const;
  int16_t               read_sample_cubic_fp() const;

  public:

  SAMPLE_PLAYER_EFFECT();
  virtual void          update() override;

  void                  play( const uint16_t* sample_data, int sample_length, float speed );
  void                  stop();

  inline bool           playing() const                 { return m_sample_data != nullptr; }
};

/////////////////////////////////////////////////////////

template< int MAX_NUM_VOICES >
class POLYPHONIC_SAMPLE_PLAYER
{
  SAMPLE_PLAYER_EFFECT* m_sample_players[ MAX_NUM_VOICES ];

  int                   m_num_voices;
  int                   m_next_voice;

  const uint16_t*       m_sample_data;
  int                   m_sample_length;

 public:

  POLYPHONIC_SAMPLE_PLAYER(const uint16_t* sample_data) :
    m_num_voices(0),
    m_next_voice(0),
    m_sample_data( sample_data + 2 ),
    m_sample_length(0)
  {
    const uint32_t header = (reinterpret_cast<const uint32_t*>(sample_data))[0];
    m_sample_length       = header & 0xFFFFFF;

    const int rate_code   = header >> 24;
    
    Serial.print( "sample length:");
    Serial.print(m_sample_length);
    Serial.print( " rate code:0x");
    Serial.println(rate_code, HEX);
  }

  void                  add_sample_player( SAMPLE_PLAYER_EFFECT& sample_player )
  {
    if( m_num_voices == MAX_NUM_VOICES )
    {
      Serial.println("Too many voices");
      return;
    }
    m_sample_players[ m_num_voices++ ] = &sample_player;
  }

  void                  play( float speed )
  {
    SAMPLE_PLAYER_EFFECT& sample_player = *m_sample_players[ m_next_voice ];
    sample_player.stop();
    sample_player.play( m_sample_data, m_sample_length, speed );

    if( ++m_next_voice == m_num_voices )
    {
      m_next_voice = 0;
    }
  }

  void                  play_at_pitch( int semitone )
  {
    // semitone 0 = 0.5x speed
    // semitone 1 = 1x speed
    // semitone 2 = 2x speed

    constexpr int semitone_offset = 12;
    const int offset_semitone = semitone - semitone_offset;
    const float speed = powf( 2.0f, offset_semitone / 12.0f );

    play( speed );
  }

  void                play_at_quantised_pitch( int semitone )
  {
    constexpr int8_t c_key[7] = {0,2,4,5,7,9,11};

    const int8_t semitone_in_octave  = semitone % 12;
    const int8_t semitone_offset     = ( semitone / 12 ) * 12;

    for( int st = 0; st < 7; ++st )
    {
      if( c_key[st] >= semitone_in_octave )
      {
        const int8_t semitone_to_play = c_key[st] + semitone_offset;
        play_at_pitch( semitone_to_play );
        return;
      }
    }
  }
};

/////////////////////////////////////////////////////////
