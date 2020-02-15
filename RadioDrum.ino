#include "Drum.h"
#include "CompileSwitches.h"

#include "Interface.h"
#include "MultiMixer.h"
#include "SamplePlayer.h"

#include "AudioSampleKick.h"
#include "AudioSampleType.h"
#include "AudioSampleReturn.h"
#include "AudioSampleFirehit.h"
#include "AudioSampleTink.h"

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

DRUM                  drum_1( reinterpret_cast<const uint16_t*>(&(AudioSampleKick[0])) );               // synthesised kick
DRUM                  drum_2( reinterpret_cast<const uint16_t*>(&(AudioSampleType[0])) );               // vintage adding machine key press
DRUM                  drum_3( reinterpret_cast<const uint16_t*>(&(AudioSampleReturn[0])) );             // vintage adding machine carriage return
DRUM                  drum_4( reinterpret_cast<const uint16_t*>(&(AudioSampleTink[0])) );               // vintage adding machine carriage return 2
DRUM                  drum_5( reinterpret_cast<const uint16_t*>(&(AudioSampleFirehit[0])) );            // hitting a cast iron fire


PATTERN               pattern_1;


MultiMixer2           drum_1_mixer;
MultiMixer2           drum_2_mixer;
MultiMixer2           drum_3_mixer;
MultiMixer2           drum_4_mixer;
MultiMixer2           drum_5_mixer;
MultiMixer5           dry_drum_mixer;
MultiMixer5           reverb_mixer;
MultiMixer6           delay_mixer;
MultiMixer3           final_mixer;

AudioEffectDelay      delay_effect;
AudioEffectFreeverb   freeverb_effect;


AudioOutputAnalog     audio_output;

AudioConnection       patch_cord_1( drum_1.voice(0), 0, drum_1_mixer, 0 );
AudioConnection       patch_cord_2( drum_1.voice(1), 0, drum_1_mixer, 1 );
AudioConnection       patch_cord_3( drum_2.voice(0), 0, drum_2_mixer, 0 );
AudioConnection       patch_cord_4( drum_2.voice(1), 0, drum_2_mixer, 1 );
AudioConnection       patch_cord_5( drum_3.voice(0), 0, drum_3_mixer, 0 );
AudioConnection       patch_cord_6( drum_3.voice(1), 0, drum_3_mixer, 1 );
AudioConnection       patch_cord_7( drum_4.voice(0), 0, drum_4_mixer, 0 );
AudioConnection       patch_cord_8( drum_4.voice(1), 0, drum_4_mixer, 1 );
AudioConnection       patch_cord_9( drum_5.voice(0), 0, drum_5_mixer, 0 );
AudioConnection       patch_cord_10( drum_5.voice(1), 0, drum_5_mixer, 1 );

AudioConnection       patch_cord_11( drum_1_mixer, 0, dry_drum_mixer, 0 );
AudioConnection       patch_cord_12( drum_2_mixer, 0, dry_drum_mixer, 1 );
AudioConnection       patch_cord_13( drum_3_mixer, 0, dry_drum_mixer, 2 );
AudioConnection       patch_cord_14( drum_4_mixer, 0, dry_drum_mixer, 3 );
AudioConnection       patch_cord_15( drum_5_mixer, 0, dry_drum_mixer, 4 );
AudioConnection       patch_cord_16( dry_drum_mixer, 0, final_mixer, 0 );

AudioConnection       patch_cord_17( drum_1_mixer, 0, reverb_mixer, 0 );
AudioConnection       patch_cord_18( drum_2_mixer, 0, reverb_mixer, 1 );
AudioConnection       patch_cord_19( drum_3_mixer, 0, reverb_mixer, 2 );
AudioConnection       patch_cord_20( drum_4_mixer, 0, reverb_mixer, 3 );
AudioConnection       patch_cord_21( drum_5_mixer, 0, reverb_mixer, 4 );
AudioConnection       patch_cord_22( reverb_mixer, 0, freeverb_effect, 0 );
AudioConnection       patch_cord_23( freeverb_effect, 0, final_mixer, 1 );

AudioConnection       patch_cord_24( drum_1_mixer, 0, delay_mixer, 0 );
AudioConnection       patch_cord_25( drum_2_mixer, 0, delay_mixer, 1 );
AudioConnection       patch_cord_26( drum_3_mixer, 0, delay_mixer, 2 );
AudioConnection       patch_cord_27( drum_4_mixer, 0, delay_mixer, 3 );
AudioConnection       patch_cord_28( drum_5_mixer, 0, delay_mixer, 4 );
AudioConnection       patch_cord_29( delay_mixer, 0, delay_effect, 0 );
AudioConnection       patch_cord_30( delay_effect, 0, final_mixer, 2 );
AudioConnection       patch_cord_31( delay_effect, 0, delay_mixer, 5 );   // feedback

AudioConnection       patch_cord_32( final_mixer, 0, audio_output, 0 );
AudioConnection       patch_cord_33( final_mixer, 1, audio_output, 1 );

volatile boolean g_triggered = false;
volatile uint32_t g_delta_time_ms = 0;

void notify_trigger()
{
  static uint32_t prev_time_ms = 0;
  const uint32_t time_ms = millis();

  g_delta_time_ms = time_ms - prev_time_ms;
  prev_time_ms = time_ms;
  
  g_triggered = true;
}

void setup()
{
  Serial.begin(9600);
#ifdef DEBUG_OUTPUT
  serial_port_initialised = true;
  while(!Serial);
#endif

  // initialise SD card
  SPI.setMOSI(7);
  SPI.setSCK(14);
  SD.begin();

  AudioMemory(80);

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

  // set mix for drum voices within each drum
  drum_1_mixer.set_gain_all_channels( drum_1.voice_mix() );
  drum_2_mixer.set_gain_all_channels( drum_2.voice_mix() );
  drum_3_mixer.set_gain_all_channels( drum_3.voice_mix() );
  drum_4_mixer.set_gain_all_channels( drum_4.voice_mix() );

  // set the level of each drum (currently all equal)
  dry_drum_mixer.set_gain_all_channels( 1.0f / dry_drum_mixer.num_channels() );

  // set the reverb
  reverb_mixer.set_gain( 0, 0.0f );   // kick drum
  reverb_mixer.set_gain( 1, 0.1f );   // add type
  reverb_mixer.set_gain( 2, 0.1f );   // add return
  reverb_mixer.set_gain( 3, 0.2f );   // add tink
  reverb_mixer.set_gain( 4, 0.4f );   // fire hit

  //set the delay
  delay_mixer.set_gain( 0, 0.0f );    // kick drum
  delay_mixer.set_gain( 1, 0.0f );    // add type
  delay_mixer.set_gain( 2, 0.4f );    // add return
  delay_mixer.set_gain( 3, 0.4f );    // add tink
  delay_mixer.set_gain( 4, 0.0f );    // fire hit
  
  delay_mixer.set_gain( 5, 0.6f );    // feed back

  delay_effect.delay( 0, 200 );

  // set master mixer
  final_mixer.set_gain_all_channels( 1.0f );

  trig_led.setup();
  trig_button.setup();

  delay(100);

  DEBUG_TEXT_LINE("Setup complete");
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

  DEBUG_TEXT_LINE(g_delta_time_ms);

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
