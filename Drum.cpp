#include "CompileSwitches.h"
#include "Util.h"

#include "Drum.h"

////////////////////////////////////////////////////////////

DRUM::DRUM( const uint16_t* sample_data ) :
  m_voices(),
  m_poly_player(sample_data)
{
  for( SAMPLE_PLAYER_EFFECT& voice : m_voices )
  {
    m_poly_player.add_sample_player( voice );
  }
}

void DRUM::trigger( int pitch, float gain )
{
  m_poly_player.play_at_pitch(pitch, gain);
}

////////////////////////////////////////////////////////////

SEQUENCE::SEQUENCE( DRUM& drum ) :
  m_drum(&drum)
{
}

bool SEQUENCE::read(File& file)
{
  m_sequence_size = 0;

  auto consume_comma_check_end = [&file]() -> bool
  {
    if( file.available() == 0 )
    {
      return true;
    }
    
    char c = file.read();
    if( c == ',' )
    {
     return false;
    }
    else if( c == '\n' )
    {
      return true;
    }
    else
    {
      DEBUG_TEXT("Expected comma or new line");
      return true;
    }
  };
  
  while( file.available() > 0 )
  {
    char c = file.read();

    if( c == '-' )
    {
      // no trigger
      m_sequence[m_sequence_size++] = { TRIGGER::EMPTY, 0 };

       DEBUG_TEXT("Trig{EMPTY} ");

       if( consume_comma_check_end() )
       {
        break;
       }
    }
    else if( c == '{' )
    {
      auto read_int = [&file](char terminator) -> int
      {
        const int BUFFER_SIZE = 256;
        char buffer[BUFFER_SIZE];
        int bi = 0;
        char c;
        while( file.available() > 0 && (c = file.read()) != terminator && bi < BUFFER_SIZE )
        {
          buffer[bi++] = c;
        }
        buffer[bi] = '\0';

        if( bi == 0 )
        {
          DEBUG_TEXT_LINE("Unable to read trigger");
          return 0;
        }
        else if( bi >= BUFFER_SIZE )
        {
          DEBUG_TEXT_LINE("Trigger too large");
          return 0;
        }
               
        return atoi(buffer);       
      };

      const int8_t pitch            = read_int(',');
      const uint8_t velocity        = read_int('}');

      m_sequence[m_sequence_size++] = { pitch, velocity };

      DEBUG_TEXT("Trig{");
      DEBUG_TEXT(pitch);
      DEBUG_TEXT(",");
      DEBUG_TEXT(velocity);
      DEBUG_TEXT("} ");

       if( consume_comma_check_end() )
       {
        break;
       }
    }
    else
    {
      DEBUG_TEXT_LINE("Unknown character at beginning of trigger");
    }
  }

  DEBUG_TEXT(m_sequence_size);
  DEBUG_TEXT_LINE(" <END>");
  return m_sequence_size > 0;
}

void SEQUENCE::clock()
{
  const TRIGGER& trig = m_sequence[m_beat];
  if( trig.m_pitch >= 0 )
  {
    // TODO use value to set pitch and volume
    m_drum->trigger(trig.m_pitch, trig.m_velocity / 255.0f);

    DEBUG_TEXT("TRIG p:");
    DEBUG_TEXT(trig.m_pitch);
    DEBUG_TEXT(" v:");
    DEBUG_TEXT_LINE(trig.m_velocity / 255.0f);
  }

  m_beat = (m_beat + 1) % m_sequence_size;
}

////////////////////////////////////////////////////////////

void PATTERN::read( const char* filename, const DRUM_SET& drums ) 
{
  File pattern_file = SD.open(filename, FILE_READ);

  if( !pattern_file )
  {
    DEBUG_TEXT("Unable to open file:");
    DEBUG_TEXT_LINE(filename);
  }

  size_t di = 0;
  for( DRUM* drum : drums )
  {
    m_sequences[di++] = SEQUENCE(*drum);
  }

  size_t num_sequences = 0;
  while( num_sequences < m_sequences.size() && m_sequences[num_sequences].read(pattern_file) )
  {
    ++num_sequences;
  }

  if( di != num_sequences )
  {
    DEBUG_TEXT_LINE("Inconsistent number of sequences and drums");
  }
}

void PATTERN::clock()
{
  for( SEQUENCE& seq : m_sequences )
  {
    seq.clock();
  }
}
