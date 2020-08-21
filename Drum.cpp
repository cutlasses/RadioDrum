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

int SEQUENCE::sequence_length() const
{
  return m_sequence_length;
}

bool SEQUENCE::read(File& file)
{
  m_sequence_length = 0;

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
      m_sequence[m_sequence_length++] = { TRIGGER::EMPTY, 0 };

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

      m_sequence[m_sequence_length++] = { pitch, velocity };

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
    else if( m_sequence_length == 0 )
    {
      DEBUG_TEXT("Trig{EMPTY} ");
      break;
    }
    else
    {
      DEBUG_TEXT_LINE("Unknown character at beginning of trigger");
      break;
    }
  }

  DEBUG_TEXT(m_sequence_length);
  DEBUG_TEXT_LINE(" <END>");
  return m_sequence_length > 0;
}

bool SEQUENCE::clock(int id)
{
  const TRIGGER& trig = m_sequence[m_beat];
  if( trig.m_pitch != TRIGGER::EMPTY )
  {
    m_drum->trigger(trig.m_pitch, trig.m_velocity / 127.0f);

    /*
    DEBUG_TEXT("TRIG id:");
    DEBUG_TEXT(id);
    DEBUG_TEXT(" p:");
    DEBUG_TEXT(trig.m_pitch);
    DEBUG_TEXT(" v:");
    DEBUG_TEXT_LINE(trig.m_velocity / 255.0f);
    */
  }

  bool cycle_complete = false;
  if( ++m_beat >= m_sequence_length )
  {
    m_beat          = 0;
    cycle_complete  = true;
  }
  
  return cycle_complete;
}

////////////////////////////////////////////////////////////

bool PATTERN::read( const char* filename, const DRUM_SET& drums ) 
{
  File pattern_file = SD.open(filename, FILE_READ);

  if( !pattern_file )
  {
    return false;
  }

  DEBUG_TEXT("Loading:")
  DEBUG_TEXT_LINE(filename);

  size_t di = 0;
  for( DRUM* drum : drums )
  {
    m_sequences[di++] = SEQUENCE(*drum);
  }

  size_t num_sequences = 0;
  int largest_sequence_length = 0;
  while( num_sequences < m_sequences.size() && m_sequences[num_sequences].read(pattern_file) )
  {
    const int sequence_length = m_sequences[num_sequences].sequence_length();
    if( sequence_length > largest_sequence_length )
    {
      largest_sequence_length = sequence_length;
      m_leading_sequence      = num_sequences;
    }
    ++num_sequences;
  }

  /*
  if( di != num_sequences )
  {
    DEBUG_TEXT_LINE("Inconsistent number of sequences and drums, some drums will not play");
  }
  */

  DEBUG_TEXT("Leading_sequence:");
  DEBUG_TEXT_LINE(m_leading_sequence);

  return true;
}

bool PATTERN::clock()
{
  bool leading_cycle_complete = true;
  int index = 0;
  for( SEQUENCE& seq : m_sequences )
  {
    bool cycle_complete = seq.clock(index);

    if( index++ == m_leading_sequence )
    {
      leading_cycle_complete = cycle_complete;
    }
  }

  return leading_cycle_complete;
}

////////////////////////////////////////////////////////////

bool PATTERN_SET::is_pattern_pending() const
{
  return m_current_pattern != m_pending_pattern;
}

int PATTERN_SET::current_pattern() const
{
  return m_current_pattern;
}

int PATTERN_SET::pending_pattern() const
{
  return m_pending_pattern;
}

void PATTERN_SET::read( const DRUM_SET& drums )
{
  const char* pattern_filenames[MAX_PATTERNS] = { "p1.txt", "p2.txt", "p3.txt", "p4.txt" };

  m_num_patterns = 0;
  for( const char* filename : pattern_filenames )
  {
    if( !m_patterns[m_num_patterns++].read( filename, drums ) )
    {
      break;
    }
  }
}

void PATTERN_SET::advance_pending_pattern()
{
  m_pending_pattern = ( m_pending_pattern + 1 ) % m_num_patterns;
}

void PATTERN_SET::clock()
{
  const bool cycle_complete = m_patterns[m_current_pattern].clock();

  if( cycle_complete && m_pending_pattern != m_current_pattern )
  {
    m_current_pattern = m_pending_pattern;
  }
}
