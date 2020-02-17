#include <Wire.h>
#include <Audio.h>
#include "Interface.h"

#define BOUNCE_TIME         10

//////////////////////////////////////

DIAL_BASE::DIAL_BASE( bool invert ) :
  m_current_value( 0 ),
  m_invert( invert )
{
  
}

DIAL_BASE::~DIAL_BASE()
{
  
}

bool DIAL_BASE::set_current_value( int new_value )
{
  constexpr int threshold( 16 );
  //if( new_value != m_current_value )
  const int diff = new_value - m_current_value;
  if( ( diff > 0 && diff > threshold ) || ( diff < 0 && diff < -threshold ) )
  {
    m_current_value = new_value;
    return true;
  }

  return false; 
}

float DIAL_BASE::value( float max_value ) const
{
  const float vf = m_current_value / max_value;

  if( m_invert )
  {
    return 1.0f - vf;
  }
  else
  {
    return vf;
  }
}

DIAL::DIAL( int data_pin ) :
  DIAL_BASE( false ),
  m_data_pin( data_pin )
{

}

bool DIAL::update()
{
  const int new_value = analogRead( m_data_pin );

  return set_current_value( new_value );
}

//////////////////////////////////////

BUTTON::BUTTON( int data_pin, bool is_toggle ) :
  m_data_pin( data_pin ),
  m_is_toggle( is_toggle ),
  m_prev_is_active( false ),
  m_is_active( false ),
  m_down_time_stamp( 0 ),
  m_down_time_curr( 0 ),
  m_bounce( m_data_pin, BOUNCE_TIME )
{
}

bool BUTTON::active() const
{
  return m_is_active;
}

bool BUTTON::single_click() const
{
  return m_is_active && !m_prev_is_active;
}

int32_t BUTTON::down_time_ms() const
{
  if( m_down_time_stamp > 0 )
  {
//#ifdef DEBUG_OUTPUT
//  Serial.print("Down time:");
//  Serial.print(m_down_time_curr);
//  Serial.print("\n");
//#endif // DEBUG_OUTPUT
    return m_down_time_curr;
  }
  else
  {
    return 0;
  }
}

void BUTTON::setup()
{
  pinMode( m_data_pin, INPUT_PULLUP );
}

void BUTTON::update( uint32_t time_ms )
{ 
  m_bounce.update();

  m_prev_is_active = m_is_active;

  if( m_bounce.risingEdge() )
  {
    // BUTTON DOWN
    if( m_is_toggle )
    {
      m_is_active = !m_is_active;
    }
    else
    {
      m_is_active = true;
    }

    // time stamp when button is pressed
    m_down_time_stamp = time_ms;
  }
  else if( m_bounce.fallingEdge() )
  {
    // BUTTON UP
    if( !m_is_toggle )
    {
      m_is_active = false;
    }

    // reset when button released
    m_down_time_stamp = 0;
  }

  if( m_down_time_stamp > 0 )
  {
    m_down_time_curr = time_ms - m_down_time_stamp;
  }
}

//////////////////////////////////////

LED::LED() :
  m_data_pin( 0 ),
  m_is_active( false ),
  m_flash_active( false ),
  m_flash_continuous( false ),
  m_analog( false ),
  m_flash_off_time_ms( 0 ),
  m_flash_duration_ms( 0 )
{
}

LED::LED( int data_pin, bool analog ) :
  m_data_pin( data_pin ),
  m_is_active( false ),
  m_flash_active( false ),
  m_flash_continuous( false ),
  m_analog( analog ),
  m_flash_off_time_ms( 0 ),
  m_flash_duration_ms( 0 )
{
}

bool LED::is_active() const
{
  return m_is_active;
}

bool LED::is_flash_active() const
{
  return m_flash_active;
}

void LED::set_active( bool active )
{
  m_is_active = active;

  if( m_is_active )
  {
    m_flash_active      = false;
    m_flash_continuous  = false;
  }
}

void LED::flash_on( uint32_t time_ms, uint32_t flash_duration_ms, bool continuous )
{
  m_flash_active      = true;
  m_flash_off_time_ms = time_ms + flash_duration_ms;

  m_is_active         = true;
  m_flash_continuous  = true;
  m_flash_duration_ms = flash_duration_ms;
}

void LED::set_brightness( float brightness )
{
  m_brightness = brightness * 255.0f;  
}

void LED::setup()
{
  pinMode( m_data_pin, OUTPUT );
}

void LED::update( uint32_t time_ms )
{
  if( m_flash_active )
  {
    if( time_ms > m_flash_off_time_ms )
    {
      if( m_flash_continuous )
      {
        // toggle flash
        m_is_active = !m_is_active;
        m_flash_off_time_ms = time_ms + m_flash_duration_ms;
      }
      else if( m_is_active )
      {
        // continuous flash
        m_is_active     = false;
        m_flash_active  = false;        
      }
    }
  }

  if( m_analog )
  {
    if( m_is_active )
    {   
      analogWrite( m_data_pin, m_brightness );
    }
    else
    {
      analogWrite( m_data_pin, 0 );
    }
  }
  else
  {
    if( m_is_active )
    {   
      digitalWrite( m_data_pin, HIGH );
    }
    else
    {
      digitalWrite( m_data_pin, LOW );
    }
  }
}

//////////////////////////////////////

PUSH_AND_TURN::PUSH_AND_TURN( const DIAL& dial, const BUTTON& button, float initial_secondary_value ) :
  m_dial( dial ),
  m_button( button ),
  m_primary_value( 0.0f ),
  m_secondary_value( initial_secondary_value ),
  m_push_and_turning( false )
{
  
}

float PUSH_AND_TURN::primary_value() const
{
  return m_primary_value;
}

float PUSH_AND_TURN::secondary_value() const
{
  return m_secondary_value;
}

void PUSH_AND_TURN::update()
{
  if( m_push_and_turning )
  {
    // keep going until button is release
    m_push_and_turning = m_button.down_time_ms() > 0; 
  }
  else
  {
    // check for start of push and turn
    if( m_button.down_time_ms() > PUSH_AND_TURN_DOWN_TIME_MS &&
        abs( m_dial.value() - m_secondary_value ) > PUSH_AND_TURN_DIAL_TOLERANCE )
    {
      m_push_and_turning = true;
    }
  }

  // if holding button AND turning dial - update secondary value
  if( m_push_and_turning )
  {
    m_secondary_value = m_dial.value();
  }
  else
  {
    m_primary_value = m_dial.value();
  }
}
