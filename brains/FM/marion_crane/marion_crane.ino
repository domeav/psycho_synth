/* 
  FMSYNTH PSYCHOBRAIN FOR PSYCHO_SYNTH!

  Carbon copy of Mozzi FMSynth example, plus button.

  knob 1 is carrier freq
  knob 2 is modulation intensity
  knob 3 is modulator speed
  button will change mod ratio
  LED lights up when button is pressed (duh)

  You need Mozzi for this sketch to work: http://sensorium.github.io/Mozzi
  
  Originally forked from Mozzi's example Knob_LightLevel_x2_FMsynth, so license is

  Tim Barrass 2013, CC by-nc-sa
  Dom Eav 2017, CC by-nc-sa
  
*/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator 
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <Smooth.h>
#include <AutoMap.h> // maps unpredictable inputs to a range

const int CONTROL_FREQ = 64;

// psycho_synth default pins
// analog
const int KNOB_MAX = 1023;
const int KNOB0_PIN = 0;
const int KNOB1_PIN=2;
const int KNOB2_PIN=4;
// digital
const int BTN_PIN=4;
const int LED_PIN=12;

// smoothing for intensity to remove clicks on transitions
const float smoothness = 0.95f;
Smooth <long> aSmoothIntensity(smoothness);

// FM params and automaps
const int MIN_CARRIER_FREQ = 22;
const int MAX_CARRIER_FREQ = 440;
AutoMap kMapCarrierFreq(0,KNOB_MAX,MIN_CARRIER_FREQ,MAX_CARRIER_FREQ);

const int MIN_INTENSITY = 10;
const int MAX_INTENSITY = 700;
AutoMap kMapIntensity(0,KNOB_MAX,MIN_INTENSITY,MAX_INTENSITY);

const int MIN_MOD_SPEED = 1;
const int MAX_MOD_SPEED = 10000;
AutoMap kMapModSpeed(0,KNOB_MAX,MIN_MOD_SPEED,MAX_MOD_SPEED);

Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kIntensityMod(COS2048_DATA);

const int mod_ratio_low = 5;
const int mod_ratio_high = 11;

// state vars
long fm_intensity;


void setup(){
  Serial.begin(115200);
  startMozzi(CONTROL_FREQ);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);
}


void updateControl(){
  // read knobs
  int KNOB0_value = mozziAnalogRead(KNOB0_PIN);
  int KNOB1_value = mozziAnalogRead(KNOB1_PIN);
  int KNOB2_value = mozziAnalogRead(KNOB2_PIN);
  
  // read the state of the pushbutton value:
  int buttonState = digitalRead(BTN_PIN);

  int mod_ratio = mod_ratio_low;
  if (buttonState == HIGH){
    mod_ratio = mod_ratio_high;
  }
  
  // map knob0
  int carrier_freq = kMapCarrierFreq(KNOB0_value);
  aCarrier.setFreq(carrier_freq);
  aModulator.setFreq(carrier_freq * mod_ratio);
  
  // map knob1
  int intensity = kMapIntensity(KNOB1_value);
  fm_intensity = ((long)intensity * (kIntensityMod.next()+128))>>8; // shift back to range after 8 bit multiply

  // map knob2
  float mod_speed = (float)kMapModSpeed(KNOB2_value)/1000;
  kIntensityMod.setFreq(mod_speed);

  if (buttonState == HIGH) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}


int updateAudio(){
  long modulation = aSmoothIntensity.next(fm_intensity) * aModulator.next();
  return aCarrier.phMod(modulation);
}


void loop(){
  audioHook();
}

