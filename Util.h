#pragma once

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>     // Arduino compiler can get confused if you don't include include all required headers in this file?!?

/////////////////////////////////////////////////////

template <typename T>
T clamp( const T& value, const T& min, const T& max )
{
  if( value < min )
  {
    return min;
  }
  if( value > max )
  {
    return max;
  }
  return value;
}

template <typename T>
T max_val( const T& v1, const T& v2 )
{
  if( v1 > v2 )
  {
    return v1;
  }
  else
  {
    return v2;
  }
}

template <typename T>
T min_val( const T& v1, const T& v2 )
{
  if( v1 < v2 )
  {
    return v1;
  }
  else
  {
    return v2;
  }
}

/////////////////////////////////////////////////////

template <typename T>
constexpr T lerp( const T& v1, const T& v2, const T& t )
{
  return v1 + ( (v2 - v1) * t );
}

// from http://polymathprogrammer.com/2008/09/29/linear-and-cubic-interpolation/
template <typename T>
inline constexpr T cubic_interpolation( T p0, T p1, T p2, T p3, T t )
{
  const T one_minus_t = T(1.0f) - t;
  return ( one_minus_t * one_minus_t * one_minus_t * p0 ) + ( T(3.0f) * one_minus_t * one_minus_t * t * p1 ) + ( T(3.0f) * one_minus_t * t * t * p2 ) + ( t * t * t * p3 );
}

/////////////////////////////////////////////////////

inline constexpr int trunc_to_int( float v )
{
  return static_cast<int>( v );
}

inline constexpr int round_to_int( float v )
{
	return static_cast<int>( v + 0.5f );
}

/////////////////////////////////////////////////////

template < typename TYPE, int CAPACITY >
class RUNNING_AVERAGE
{
  TYPE                    m_values[ CAPACITY ];
  int                     m_current;
  int                     m_size;

public:

  RUNNING_AVERAGE() :
    m_values(),
    m_current(0),
    m_size(0)
  {
  }

  void add( TYPE value )
  {
    m_values[ m_current ] = value;
    m_current             = ( m_current + 1 ) % CAPACITY;
    ++m_size;
    if( m_size > CAPACITY )
    {
      m_size              = CAPACITY;
    }
  }

  void reset()
  {
    m_size                = 0;
    m_current             = 0;
  }
  
  TYPE average() const
  {
    if( m_size == 0 )
    {
      return 0;  
    }
    
    TYPE avg = 0;
    for( int x = 0; x < m_size; ++x )
    {
      avg += m_values[ x ];
    }

    return avg / m_size;
  }

  int size() const
  {
    return m_size;
  }
};

inline float random_ranged( float min, float max )
{
	const float range = max - min;
	return ( ( static_cast<float>( rand() ) / RAND_MAX ) * range ) + min;
}

/////////////////////////////////////////////////////
