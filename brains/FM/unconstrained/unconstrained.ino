/* 
  FMSYNTH PSYCHOBRAIN FOR PSYCHO_SYNTH!

  Experiment with unconstrained ratio between carrier and modulator, so this will be noisy.
  Easy on your ears!!!

  knob 1 is carrier freq
  knob 2 is modulation intensity
  knob 3 is modulator freq
  button is modulation speed (keep pressed, then release)
  LED blinks depending on mod speed

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

const int MIN_MOD_FREQ = 22;
const int MAX_MOD_FREQ = 4400;
AutoMap kMapModFreq(0,KNOB_MAX,MIN_MOD_FREQ,MAX_MOD_FREQ);


Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kIntensityMod(COS2048_DATA);


// state vars
long fm_intensity = 0;
int buttonState = 0;
int nbCycles = 0;
float mod_speed = 0.2f;
int ledCounter = 0;
int ledCycleLength = 320; // for initial mod_speed

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
  Serial.print("KNOB0 = "); 
  Serial.print(KNOB0_value);
  Serial.print("\t");
  Serial.print("KNOB1 = "); 
  Serial.print(KNOB1_value);
  Serial.print("\t");
  Serial.print("KNOB2 = "); 
  Serial.print(KNOB2_value);
  Serial.print("\t");
    
  // map knob0
  int carrier_freq = kMapCarrierFreq(KNOB0_value);
  aCarrier.setFreq(carrier_freq);

  // map knob1
  int intensity = kMapIntensity(KNOB1_value);
  fm_intensity = ((long)intensity * (kIntensityMod.next()+128))>>8; // shift back to range after 8 bit multiply

  // map knob2
  int mod_freq = kMapModFreq(KNOB2_value);
  aModulator.setFreq(mod_freq);



  // read the state of the pushbutton value:
  buttonState = digitalRead(BTN_PIN);

  Serial.print(nbCycles);
  Serial.print("\t");

  if (buttonState == HIGH) {
    nbCycles++;
  } else {
    if (nbCycles != 0){
      mod_speed = (float)CONTROL_FREQ / nbCycles;
      ledCycleLength = nbCycles;
      nbCycles = 0;
    }
  }
  kIntensityMod.setFreq(mod_speed);

  if (ledCounter == 0){
    digitalWrite(LED_PIN, HIGH);
  }
  if (ledCounter == 1){
    digitalWrite(LED_PIN, LOW);
  }
  ledCounter++;
  if (ledCounter >= ledCycleLength){
    ledCounter = 0;
  }
  
  Serial.println();
}


int updateAudio(){
  long modulation = aSmoothIntensity.next(fm_intensity) * aModulator.next();
  return aCarrier.phMod(modulation);
}


void loop(){
  audioHook();
}





