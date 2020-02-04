#pragma once

#include <stdint.h>

class FIXED_POINT
{
    static constexpr int SHIFT_BITS			= 8;
    static constexpr int SHIFTED_SCALE		= 1 << SHIFT_BITS;
    static constexpr float SCALE_FACTOR_F	= static_cast<float>( 1 << SHIFT_BITS );

    int32_t			m_fp_value;

    // set the implementation value directly
    explicit constexpr FIXED_POINT( int32_t value ) :
      m_fp_value( value )
    {
    }

  public:

    FIXED_POINT() = default;
    constexpr FIXED_POINT( const FIXED_POINT& rhs ) = default;
    constexpr FIXED_POINT( FIXED_POINT&& rhs ) = default;
    constexpr FIXED_POINT& operator=( const FIXED_POINT& rhs ) = default;

    constexpr FIXED_POINT& operator=( int16_t rhs )
    {
      m_fp_value = (rhs << SHIFT_BITS);
      return *this;
    }

    explicit constexpr inline FIXED_POINT( float value ) :
      m_fp_value( static_cast<int32_t>( (value * SCALE_FACTOR_F) + 0.5f ) )
    {
    }

    explicit constexpr inline FIXED_POINT( int16_t value ) :
      m_fp_value( value << SHIFT_BITS )
    {
    }

    explicit constexpr inline FIXED_POINT( uint16_t value ) :
      m_fp_value( value << SHIFT_BITS )
    {
    }

    inline constexpr float to_float() const
    {
      return static_cast<float>( m_fp_value ) / SCALE_FACTOR_F;
    }

    inline constexpr int16_t trunc_to_int16() const
    {
      return static_cast<int16_t>(m_fp_value >> SHIFT_BITS);
    }

    inline constexpr uint16_t trunc_to_uint16() const
    {
      return static_cast<uint16_t>(m_fp_value >> SHIFT_BITS);
    }

    inline constexpr int32_t trunc_to_int32() const
    {
      return (m_fp_value >> SHIFT_BITS);
    }

    inline constexpr int16_t round_to_int() const
    {
      return static_cast<int16_t>(to_float() + 0.5f);
    }

    inline constexpr bool operator == ( const FIXED_POINT& rhs ) const
    {
      return m_fp_value == rhs.m_fp_value;
    }

    inline constexpr bool operator < ( const FIXED_POINT& rhs ) const
    {
      return m_fp_value < rhs.m_fp_value;
    }

    inline constexpr bool operator <= ( const FIXED_POINT& rhs ) const
    {
      return m_fp_value <= rhs.m_fp_value;
    }

    inline constexpr bool operator > ( const FIXED_POINT& rhs ) const
    {
      return m_fp_value > rhs.m_fp_value;
    }

    inline constexpr bool operator >= ( const FIXED_POINT& rhs ) const
    {
      return m_fp_value >= rhs.m_fp_value;
    }

    inline constexpr FIXED_POINT operator + ( const FIXED_POINT& rhs ) const
    {
      return FIXED_POINT( m_fp_value + rhs.m_fp_value );
    }

    inline constexpr FIXED_POINT operator - ( const FIXED_POINT& rhs ) const
    {
      return FIXED_POINT( m_fp_value - rhs.m_fp_value );
    }

    inline constexpr FIXED_POINT operator * ( const FIXED_POINT& rhs ) const
    {
      // NOTE: this can overflow (cast to int64 to fix if required
      if ( m_fp_value >= 0 &&  rhs.m_fp_value >= 0 )
      {
        return FIXED_POINT( (m_fp_value * rhs.m_fp_value) >> SHIFT_BITS );
      }
      // can't shift a negative number
      else
      {
        if ( m_fp_value < 0 )
        {
          if ( rhs.m_fp_value < 0 )
          {
            // both are negative
            const int32_t new_value = -m_fp_value * -rhs.m_fp_value;
            return FIXED_POINT( new_value >> SHIFT_BITS );
          }
          else
          {
            // only this is negative
            const int32_t new_value = (-m_fp_value) * rhs.m_fp_value;
            return FIXED_POINT( (-new_value) >> SHIFT_BITS );
          }
        }
        else
        {
          // only rhs is negative
          const int32_t new_value = m_fp_value * (-rhs.m_fp_value);
          return FIXED_POINT( (-new_value) >> SHIFT_BITS );
        }
      }
    }

    inline constexpr FIXED_POINT operator / ( const FIXED_POINT& rhs ) const
    {
      // NOTE: this can overflow (cast to int64 to fix if required
      return FIXED_POINT( (m_fp_value * SHIFTED_SCALE) / rhs.m_fp_value );
    }

    inline constexpr void operator += ( const FIXED_POINT& rhs )
    {
      m_fp_value += rhs.m_fp_value;
    }

    inline friend constexpr FIXED_POINT operator * ( int16_t lhs, const FIXED_POINT& rhs )
    {
      return FIXED_POINT( lhs ) * rhs;
    }

    inline friend constexpr FIXED_POINT operator - ( const FIXED_POINT& lhs, int32_t rhs )
    {
      return lhs - FIXED_POINT( rhs << SHIFT_BITS );
    }
};
