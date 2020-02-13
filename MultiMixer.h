#pragma once

#include <Audio.h>

#include "util.h"

// based on Teensy audio library AudioMixer4
// NOTE currently not optimised for Teensy 4
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
  
  void gain(unsigned int channel, float gain)
  {
    if( channel >= NUM_CHANNELS )
    {
      return;
    }

    gain = clamp( gain, -127.0f, 127.0f );

    m_channel_mults[channel] = gain * UNITY_GAIN;
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

using MultiMixer4 = MULTI_MIXER<4>;

