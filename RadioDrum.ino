#include "Drum.h"
#include "CompileSwitches.h"

#include "Interface.h"
#include "MultiMixer.h"
#include "SamplePlayer.h"

#include "AudioSampleKick.h"
#include "AudioSampleAddhit1.h"
#include "AudioSampleAddreturn.h"
#include "AudioSampleFirehit.h"
#include "AudioSamplePop.h"

constexpr int         NOTE_CV_PIN(A8);    // ROOT - on panel
constexpr int         TRIG_CV_PIN(9);     // TRIG - on panel
constexpr int         TRIG_BUTTON_PIN(8);
constexpr int         RESET_LED_PIN(11);
constexpr int         CHORD_POT_PIN(A9);
constexpr int         ROOT_POT_PIN(A7);
constexpr int         ADC_BITS(13);
constexpr int         ADC_MAX_VAL(8192);

constexpr int         TRIG_FLASH_TIME_MS(100);

LED                   trig_led(RESET_LED_PIN, false);
BUTTON                trig_button(TRIG_BUTTON_PIN, false);
DIAL                  root_dial(ROOT_POT_PIN);
DIAL                  chord_dial(CHORD_POT_PIN);

DRUM                  drum_1( reinterpret_cast<const uint16_t*>(&(AudioSampleKick[0])) );
DRUM                  drum_2( reinterpret_cast<const uint16_t*>(&(AudioSampleAddhit1[0])) );
DRUM                  drum_3( reinterpret_cast<const uint16_t*>(&(AudioSampleAddreturn[0])) );
DRUM                  drum_4( reinterpret_cast<const uint16_t*>(&(AudioSampleFirehit[0])) );
DRUM                  drum_5( reinterpret_cast<const uint16_t*>(&(AudioSamplePop[0])) );

PATTERN               pattern_1;


MultiMixer4           drum_1_mixer;
MultiMixer4           drum_2_mixer;
MultiMixer4           drum_3_mixer;
MultiMixer4           drum_4_mixer;
MultiMixer4           master_drum_mixer;

AudioOutputAnalog     audio_output;

AudioConnection       patch_cord_1( drum_1.voice(0), 0, drum_1_mixer, 0 );
AudioConnection       patch_cord_2( drum_1.voice(1), 0, drum_1_mixer, 1 );
AudioConnection       patch_cord_3( drum_2.voice(0), 0, drum_2_mixer, 0 );
AudioConnection       patch_cord_4( drum_2.voice(1), 0, drum_2_mixer, 1 );
AudioConnection       patch_cord_5( drum_3.voice(0), 0, drum_3_mixer, 0 );
AudioConnection       patch_cord_6( drum_3.voice(1), 0, drum_3_mixer, 1 );
AudioConnection       patch_cord_7( drum_4.voice(0), 0, drum_4_mixer, 0 );
AudioConnection       patch_cord_8( drum_4.voice(1), 0, drum_4_mixer, 1 );
AudioConnection       patch_cord_9( drum_1_mixer, 0, master_drum_mixer, 0 );
AudioConnection       patch_cord_10( drum_2_mixer, 0, master_drum_mixer, 1 );
AudioConnection       patch_cord_11( drum_3_mixer, 0, master_drum_mixer, 2 );
AudioConnection       patch_cord_12( drum_4_mixer, 0, master_drum_mixer, 3 );
AudioConnection       patch_cord_13( master_drum_mixer, 0, audio_output, 0 );
AudioConnection       patch_cord_14( master_drum_mixer, 1, audio_output, 1 );

volatile boolean g_triggered = false;

void notify_trigger()
{
  g_triggered = true;
}

void setup()
{
  Serial.begin(9600);
#ifdef DEBUG_OUTPUT
  serial_port_initialised = true;
  //while(!Serial);
#endif

  // initialise SD card
  SPI.setMOSI(7);
  SPI.setSCK(14);
  SD.begin();

  AudioMemory(64);

  // RADIO MUSIC setup
  analogReference(DEFAULT);
  analogReadRes(ADC_BITS);
  pinMode( NOTE_CV_PIN, INPUT );
  pinMode( TRIG_CV_PIN, INPUT );

  // Add an interrupt on the RESET_CV pin to catch rising edges
  attachInterrupt( digitalPinToInterrupt(TRIG_CV_PIN), notify_trigger, RISING );

  DRUM_SET drums;
  drums[0] = &drum_1;
  drums[1] = &drum_2;
  drums[2] = &drum_3;
  drums[3] = &drum_4;
  drums[4] = &drum_5;
  pattern_1.read("p1.txt", drums);

  auto setup_mix =[](MultiMixer4& mixer, int num_channels, float gain)
  {
    for( int ci = 0; ci < num_channels; ++ci )
    {
      drum_1_mixer.gain(ci, gain );
    }    
  };

  setup_mix( drum_1_mixer, drum_1.num_voices_per_drum(), drum_1.voice_mix() );

  setup_mix( drum_4_mixer, 4, 0.25f );

  trig_led.setup();
  trig_button.setup();

  delay(100);

  DEBUG_TEXT("Setup complete");
}

void loop()
{
  const int time = millis();
  
  trig_led.update( time );
  trig_button.update( time );
  
  if( trig_button.single_click() )
  {
    g_triggered = true;
  }
  
  if( g_triggered )
  {
    g_triggered = false;

    pattern_1.clock();

    trig_led.flash_on( time, TRIG_FLASH_TIME_MS );
  }

#ifdef SHOW_PERF
  int perf_time = millis();
  static int time_stamp = 0;
  static int total_processor_usage = 0;
  static int num_samples = 0;
  if( perf_time > time_stamp )
  {
    time_stamp = perf_time + 1000.0f;
    
    Serial.print( "Average processor usage:" );
    Serial.println( static_cast<float>(total_processor_usage) / num_samples );

    total_processor_usage = 0;
    num_samples = 0;
  }
  else
  {
    total_processor_usage += AudioProcessorUsage();
    ++num_samples;
  }
#endif // SHOW_PERF
}
