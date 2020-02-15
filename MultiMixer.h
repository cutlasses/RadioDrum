#pragma once

#include <Audio.h>

#include "util.h"

// based on Teensy audio library AudioMixer4
// NOTE there appears to be a more efficient version
template<int32_t NUM_CHANNELS>
class MULTI_MIXER : public AudioStream
{
public:

  MULTI_MIXER() :
    AudioStream(NUM_CHANNELS, m_input_queue_array)
  {
    for( int16_t& mult : m_channel_mults )
    {
      mult = UNITY_GAIN;
    }
  }

  virtual void update(void) override
  {
    audio_block_t* out = nullptr;

    for( int channel = 0; channel < NUM_CHANNELS; ++channel )
    {
      if( out == nullptr )
      {
        out = receiveWritable(channel);
        if( out != nullptr )
        {
          int32_t mult = m_channel_mults[channel];
          if( mult != UNITY_GAIN )
          {
            apply_gain( out->data, mult );
          }
        }
      }
      else
      {
        audio_block_t* in = receiveReadOnly(channel);
        if( in != nullptr )
        {
          apply_gain_then_add( in->data, out->data, m_channel_mults[channel] );
          release( in );
        }
      }
    }

    if( out != nullptr )
    {
      transmit(out);
      release(out);
    }
  }
  
  void set_gain(int32_t channel, float gain)
  {
    if( channel >= NUM_CHANNELS )
    {
      DEBUG_TEXT_LINE("Invalid channel");
      return;
    }

    gain = clamp( gain, -127.0f, 127.0f );

    m_channel_mults[channel] = gain * UNITY_GAIN;
  }

  void set_gain_all_channels( float gain )
  {
    for( int32_t channel = 0; channel < NUM_CHANNELS; ++channel )
    {
      set_gain( channel, gain );
    }
  }

  constexpr int32_t num_channels() const
  {
    return NUM_CHANNELS;
  }
  
private:

  static const constexpr int UNITY_GAIN = 256;

  void apply_gain(int16_t* dst, int32_t mult)
  {
    const int16_t* end = dst + AUDIO_BLOCK_SAMPLES;

    do
    {
      const int32_t val = (*dst * mult) >> 8;
      *dst++ = signed_saturate_rshift(val, 16, 0);
    } while( dst < end );
  }

  void apply_gain_then_add(const int16_t* src, int16_t* dst, int32_t mult)
  {
    const int16_t* end = dst + AUDIO_BLOCK_SAMPLES;
    
    if( mult == UNITY_GAIN )
    {
      do
      {
        const int32_t val = *dst + *src++;
        *dst++ = signed_saturate_rshift(val, 16, 0);
      } while( dst < end );
    }
    else
    {
      do
      {
        const int32_t val = *dst + ((*src++ * mult) >> 8);
        *dst++ = signed_saturate_rshift(val, 16, 0);
      } while( dst < end );
    }
  }

  int16_t             m_channel_mults[NUM_CHANNELS];
  audio_block_t*      m_input_queue_array[NUM_CHANNELS];
};

using MultiMixer2 = MULTI_MIXER<2>;
using MultiMixer3 = MULTI_MIXER<3>;
using MultiMixer4 = MULTI_MIXER<4>;
using MultiMixer5 = MULTI_MIXER<5>;
using MultiMixer6 = MULTI_MIXER<6>;
