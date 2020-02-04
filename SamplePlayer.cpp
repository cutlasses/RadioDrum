#include "Util.h"
#include "SamplePlayer.h"

constexpr FIXED_POINT FIXED_POINT_ZERO( 0.0f );
constexpr FIXED_POINT FIXED_POINT_HALF( 0.5f );
constexpr FIXED_POINT FIXED_POINT_TWO( 2.0f );

SAMPLE_PLAYER_EFFECT::SAMPLE_PLAYER_EFFECT() :
  AudioStream( 1, m_input_queue_array ),
  m_input_queue_array(),
  m_sample_data(nullptr),
  m_sample_length(0),
  m_speed(1.0f),
  m_read_head(0.0f)
{
}

int16_t SAMPLE_PLAYER_EFFECT::read_sample_linear_fp() const
{
  // linearly interpolate between the current sample and its neighbour
  // (previous neighbour if frac is less than 0.5, otherwise next)
  const int int_part   = m_read_head.trunc_to_int32();
  const FIXED_POINT frac_part( m_read_head - int_part );
  
  const int16_t curr_samp   = m_sample_data[ int_part ];
  
  if( frac_part < FIXED_POINT_HALF )
  {
    int prev        = int_part - 1;
    if( prev < 0 )
    {
      // at the beginning of the buffer, assume next sample was the same and use that (e.g. no interpolation)
      return curr_samp;
    }
    
    const FIXED_POINT t     = frac_part * FIXED_POINT_TWO;
    
    const int16_t prev_samp = m_sample_data[ prev ];

    FIXED_POINT lerp_samp   = lerp<FIXED_POINT >( FIXED_POINT(prev_samp), FIXED_POINT(curr_samp), t ); 
        
    return lerp_samp.trunc_to_int16();
  }
  else
  {
    int next        = int_part + 1;
    if( next >= m_sample_length )
    {
      // at the end of the buffer, assume next sample was the same and use that (e.g. no interpolation)
      return curr_samp;
    }
    
    const FIXED_POINT t     = ( frac_part - FIXED_POINT_HALF ) * FIXED_POINT_TWO;
    
    const int16_t next_samp = m_sample_data[ next ];
    
    FIXED_POINT lerp_samp   = lerp<FIXED_POINT>( FIXED_POINT(curr_samp), FIXED_POINT(next_samp), t );
     
    return lerp_samp.trunc_to_int16();
  }
}

int16_t SAMPLE_PLAYER_EFFECT::read_sample_cubic_fp() const
{
  const int int_part   = m_read_head.trunc_to_int32();
  const FIXED_POINT frac_part( m_read_head - int_part );
  
  FIXED_POINT p0;
  if( int_part >= 2 )
  {
    p0                        = m_sample_data[ int_part - 2 ];
  }
  else
  {
    // at the beginning of the buffer, assume previous sample was the same
    p0                        = m_sample_data[ 0 ];
  }
  
  FIXED_POINT p1;
  if( int_part <= 2 )
  {
    // reuse p0
    p1                        = p0;
  }
  else
  {
    p1                        = m_sample_data[ int_part - 1 ];
  }
  
  FIXED_POINT p2;
  p2                          = m_sample_data[ int_part ];
  
  FIXED_POINT p3;
  if( int_part < m_sample_length - 1)
  {
    p3                        = m_sample_data[ int_part + 1 ];
  }
  else
  {
    p3                        = p2;
  }
  
  const FIXED_POINT t         = lerp<FIXED_POINT>( FIXED_POINT(0.33333f), FIXED_POINT(0.66666f), frac_part );
  
  const FIXED_POINT sampf     = cubic_interpolation<FIXED_POINT>( p0, p1, p2, p3, t );
  
  return sampf.trunc_to_int16();
}
  
void SAMPLE_PLAYER_EFFECT::update()
{
  if( playing() )
  {
    audio_block_t* block = allocate();

    if( block != nullptr )
    {
      //Serial.print("*Speed:");
      //Serial.print(m_speed.to_float());
      //Serial.print(" Read head:");
      //Serial.println(m_read_head.to_float());
      for( int i = 0; i < AUDIO_BLOCK_SAMPLES; ++i )
      {
        const int head_int = m_read_head.trunc_to_int32();
        if( head_int < m_sample_length )
        {
          block->data[i] = read_sample_cubic_fp();
          m_read_head += m_speed;
        }
        else
        {
          // reached the end of the sample
          memset( block->data + i, 0, (AUDIO_BLOCK_SAMPLES - i) * sizeof(int16_t) );
          break;
        }
      }

      transmit( block, 0 );

      release( block );
    }
  }
}

void SAMPLE_PLAYER_EFFECT::play( const uint16_t* sample_data, int sample_length, float speed )
{
  m_sample_data   = sample_data;
  m_sample_length = sample_length;
  m_speed         = FIXED_POINT(speed);
  m_read_head     = FIXED_POINT_ZERO;
}

void SAMPLE_PLAYER_EFFECT::stop()
{
  m_sample_data   = nullptr;
  m_sample_length = 0;
  m_read_head     = FIXED_POINT_ZERO;
}

