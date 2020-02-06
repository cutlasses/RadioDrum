#include "Drum.h"
#include "Interface.h"
#include "SamplePlayer.h"
#include "AudioSamplePiano_c3_44k.h"

//#define SHOW_PERF

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

DRUM                  drum_1( reinterpret_cast<const uint16_t*>(&(AudioSamplePiano_c3_44k[0])) );

AudioMixer4           drum_1_mixer;

AudioOutputAnalog     audio_output;

AudioConnection       patch_cord_1( drum_1.voice(0), 0, drum_1_mixer, 0 );
AudioConnection       patch_cord_2( drum_1.voice(1), 0, drum_1_mixer, 1 );
AudioConnection       patch_cord_3( drum_1_mixer, 0, audio_output, 0 );

volatile boolean g_triggered = false;

void notify_trigger()
{
  g_triggered = true;
}

void setup()
{
  Serial.begin(9600);

  AudioMemory(64);

  // RADIO MUSIC setup
  analogReference(DEFAULT);
  analogReadRes(ADC_BITS);
  pinMode( NOTE_CV_PIN, INPUT );
  pinMode( TRIG_CV_PIN, INPUT );

  // Add an interrupt on the RESET_CV pin to catch rising edges
  attachInterrupt( digitalPinToInterrupt(TRIG_CV_PIN), notify_trigger, RISING );

  drum_1_mixer.gain( 0, drum_1.voice_mix() );

  trig_led.setup();
  trig_button.setup();

  delay(100);
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
