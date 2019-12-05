/*
  For 5 cap touch pins on an UNO,
  where the leads are long, and thus act like an antenna (and high cap-value),
  and the ends are objects with large surface.

  This sketch based on the behavior I saw when I had long leads taped to a table
  with some objects on the end.
  The long-leads act like antennas, and pick up the 60Hz power signal.
  So, I track the peak during each 1/60 sec and use that as the cap-value.
  When I touched the _bare_ conductor, the counts jumped to 1024, from about 300.
  But, also, the untouched value would move around a bit depending on where my arm was, etc,
  so the common-mode lead (A5) is used as a reference: assume "touched" is about 70% of the
  difference from common-mode to 1024.

  Sends 0+ (1+ 2+ etc) as each pin is touched, and 0-, 1-, etc when released.
  Intended for use with processing.
  Debounce might be needed, check behavior.

  Wiring:
    Each pin A0..A4
    10Mohm gnd->pin. pin->lead->object. Might be able to change this to get a larger jump on touch.
    Pin A5 is the same, but no object, we use it as a reference.

  Calibration:
    Several values are empirically determined.
    Run the loop() with only the plot_loop() enabled, and check the behavior.
    Key things are the TouchedValue and on_threshold_ratio
*/

#include "tired_of_serial.h"
#include "HzMax.h"
#include "CrossOverDetect.h"

// magic for array size:
template <typename T, unsigned S> inline constexpr unsigned arraysize(const T (&v)[S]) {
  return S;
};

// THINGS TO SET

// Might adjust the time-window that we look for a max during. Longer shouldn't break it,
// though it might make it a bit slower to respond to changes.
// At least 1/60 sec so we try to catch at least on peak.
// Beta is the amount of averaging, roughly n samples averaged together.
const int HzPeriod = 1 / 60.0 * 1000 * 1.1; //rounded is fine. make sure we catch at least 1 peak
const int HzMaxBeta = 5; // we don't seem to need much averaging

// Pins. Just list 'em
int Touches[] = { A0, A1, A2}; // , A3, A4 }; // touch pins to scan (automagic length detect)

// Each pin behaves differently, so determine the "delta" empirically (cf. plot_xover_loop):
// Delta in plot_xover_loop is about double what it should be in touched_xover_loop()
int CrossoverDelta[] = { 90, 90, 90, 200, 200 }; // seems to get weaker A0-->>A2
static_assert(arraysize(Touches) <= arraysize(CrossoverDelta), "Touches list and CrossoverDelta list must be same size");

const int Common = A5; // un attached reference antenna. not used in this sketch

constexpr int TouchesCount = arraysize(Touches);
const char TouchLetter = '0'; // A0='0', A1='1', etc. Sent to serial port

// Our long-lead touch wire acts like an antenna and picks up the 60Hz power
// So, the period is 1/60 *1000ms * 1.1fuzz
// "commonmode" serves as a reference base value
int on_threshold; // dynamically from commonmode

void setup() {
  Serial.begin(115200);
  println("start");

  xover_setup();

  println("setup done");
}

void loop() {
  //plot_a0_loop(); // raw
  //plot_loop(); // see the values we are testing against, esp on_threshold

  //plot_on_loop(); // watch the touch as a graph, easier to see
  //touched_loop();

  // using crossover
  plot_xover_loop();
  //touched_xover_loop();
}

CrossOverDetect<HzMax, ExponentialSmooth> *xover[ TouchesCount ];
const int XOverSlowBeta = 20; // some multiple of HzMaxBeta
void xover_setup() {
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    // for track peak
    HzMax *hzmax = new HzMax( HzPeriod, HzMaxBeta );
    // for track slow peak
    ExponentialSmooth *slowMax = new ExponentialSmooth( XOverSlowBeta ); // we'll have to update this as xover[].v2.
    // add to crossover detect
    xover[pin_i] = new CrossOverDetect<HzMax, ExponentialSmooth>( CrossoverDelta[pin_i], hzmax, slowMax );
  }
}

void read_xover_pins() {
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    int v = analogRead( Touches[pin_i] );
    // for track peak
    int peak = xover[pin_i]->v1.update( v );
    // for the slow follower, for crossover
    xover[pin_i]->v2.update( peak );

    xover[pin_i]->state(); // need to calculate state
  }
}

void plot_xover_loop() {
  // This is a problem. I was hoping we could plot the values and eyeball things
  // But, it seems like printing a lot (i.e. serial), causes the analogRead() to behave differently: about x2.
  // Possibly some kind of uart/interrupt/clock thing?
  // So, values seen in this function WON'T be the same as seen w/o a lot of printing (e.g. touched_xover_loop)

  read_xover_pins();
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    //if ( !(pin_i == 0 || pin_i==4) ) continue; // skip most

    auto &pinx = *xover[pin_i]; // CrossoverDetect
    int vfast = pinx.v1.value();
    int vslow = pinx.v2.value();

    if ( pinx.changed() ) {
        print(pinx.on() ? (int)(vfast * 1.3) : vfast);print(F(" "));print(pinx.off() ? (int)(vslow * 0.6) : vslow);print(F(" "));  // peak,slow
        print(pinx.delta);print(F(" "));print(vfast - vslow);print(F(" ")); // delta, actual-delta

    }
    else {
        print(vfast);print(F(" "));print(vslow);print(F(" ")); // peak,slow
        print(pinx.delta);print(F(" "));print(vfast - vslow);print(F(" ")); // delta, actual-delta

    }
  }
  println();
}

void touched_xover_loop() {
  // the described protocol, sends "0+" etc when touched
  read_xover_pins();

  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    auto &pinx = *xover[pin_i]; // CrossoverDetect

    // debug values
    if (false && pin_i==2) {  // can disable in production
      if (pinx.current_delta() > 100) {
        print(pinx.delta);print(F(" "));print(pinx.current_delta());print(F(" ")); print(pinx.on()); println();
      } 
    }

    if ( pinx.changed() ) {
      print( (char) (TouchLetter + pin_i) ); print(pinx.on() ? "+" : "-"); println();
    }
  }
}

void plot_a0_loop() {
  print( analogRead(A0) ); println();

}

void timeread_loop() {
  // development: to check that read_pins() isn't too slow.
  unsigned long top = millis();
  for (int i = 0; i < 1000; i++) {
    read_xover_pins();
  }
  println((millis() - top) / 1000.0);
}
