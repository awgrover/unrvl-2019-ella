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
#include "OnChange.h"
#include "CrossOverDetect.h"

// magic for array size:
template <typename T, unsigned S> inline constexpr unsigned arraysize(const T (&v)[S]) {
  return S;
};

// THINGS TO SET

// To determine value: use  plot_loop() and adjust till the commonmode value is near the other pins.
const float commonmode_ratio = 1.8; // the commonmode antenna is "smaller" than the real ones, about 30%

// To determine value: use plot_loop() and see what happens on touch.
const int TouchedValue = 230; // empirically

// To determine value: use plot_loop() and see where the threshold line compares with touched values
// threshold is the last value.
// Needs to be large enough that environment changes (your arm) don't exceed the threshold,
// but small enough that a touch still works.
const float on_threshold_ratio = .06; // of the TouchedValue, to count as touched. 1/2 isn't enough. .7 is unstable

// Might adjust the time-window that we look for a max during. Longer shouldn't break it,
// though it might make it a bit slower to respond to changes.
// At least 1/60 sec so we try to catch at least on peak.
// Beta is the amount of averaging, roughly n samples averaged together.
const int HzPeriod = 1 / 60.0 * 1000 * 1.1; //rounded is fine. make sure we catch at least 1 peak
const int HzMaxBeta = 5; // we don't seem to need much averaging

// Pins
int Touches[] = { A0, A1, A2, A3, A4 }; // touch pins to scan (automagic length detect)
const int Common = A5; // un attached reference antenna

constexpr int TouchesCount = arraysize(Touches);
const char TouchLetter = '0'; // A0='0', A1='1', etc. Sent to serial port
HzMax *hzmax[ TouchesCount ];
OnChange *on_change[ TouchesCount ];

// Our long-lead touch wire acts like an antenna and picks up the 60Hz power
// So, the period is 1/60 *1000ms * 1.1fuzz
// "commonmode" serves as a reference base value
HzMax commonmode( HzPeriod, 20); // a beta to smooth it a lot
int on_threshold; // dynamically from commonmode

void setup() {
  Serial.begin(115200);
  //hzmax_setup();
  xover_setup();
}

void hzmax_setup() {
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    hzmax[pin_i] = new HzMax( HzPeriod, HzMaxBeta );
    on_change[pin_i] = new OnChange();
  }

  // maybe readpins() for 1 sec to stabilize?
}

void loop() {
  //plot_a0_loop(); // raw
  //plot_loop(); // see the values we are testing against, esp on_threshold
  plot_xover_loop();
  //plot_on_loop(); // watch the touch as a graph, easier to see
  //touched_loop();
}

CrossOverDetect<HzMax, ExponentialSmooth> *xover[ TouchesCount ];
const int XOverSlowBeta = 20; // some multiple of HzMaxBeta
void xover_setup() {
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    // track peak
    HzMax *hzmax = new HzMax( HzPeriod, HzMaxBeta );
    // track slow peak
    ExponentialSmooth *slowMax = new ExponentialSmooth( XOverSlowBeta ); // we'll have to update this as xover[].v2.
    // add to crossover detect
    xover[pin_i] = new CrossOverDetect<HzMax, ExponentialSmooth>( hzmax, slowMax );
  }
}

void plot_xover_loop() {
}

void touched_loop() {
  // the described protocol, sends "0+" etc when touched
  read_pins();
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    boolean on = hzmax[pin_i]->value() > on_threshold;
    if ( on_change[pin_i]->changed(on) ) {
      print( (char) (TouchLetter + pin_i) ); print(on ? "+" : "-"); println();
    }
  }
}

void plot_on_loop() {
  // 2nd most useful plotting for debugging, just to see the touched/release happen
  read_pins();
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    //for (int pin_i=2; pin_i<3; pin_i++) {
    boolean on = hzmax[pin_i]->value() > on_threshold;
    //print( on_threshold - hzmax[pin_i]->value() );print(" ");
    print( on * 50 + pin_i * 2 ); print(" "); // 0|10 with offset so we can see them
  }
  print("-2 "); print( 50 + TouchesCount * 2); print(" ");
  //print("1026 ");
  //print(on_threshold);print(" ");
  println();
}

void plot_a0_loop() {
  print( analogRead(A0) ); println();

}

void plot_loop() {
  // most useful plotting for debugging/calibrating.
  // Order of plot:
  // commonmode, a0,a1,a2,a3,a4, 1026-mark, threshold-for-on
  read_pins();
  print(commonmode.value()); print(" ");
  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {

    print(hzmax[pin_i]->value());
    print(" ");
  }
  //print("300 ");
  print(on_threshold);print(" ");
  println();
}

void timeread_loop() {
  // development: to check that read_pins() isn't too slow.
  unsigned long top = millis();
  for (int i = 0; i < 1000; i++) {
    read_pins();
  }
  println((millis() - top) / 1000.0);
}

void read_pins() {
  // read 'em all
  // Takes about 0.58ms to read 5! 0.10ms each!
  // And about .8ms +commonmode+calculation.
  // 60Hz is about 16ms. so we should be reasonably ok

  // adjust on_threshold based on commonmode
  int common_value = commonmode.update( commonmode_ratio * analogRead(Common) );
  on_threshold = 90; // (TouchedValue + common_value ) * on_threshold_ratio;

  for (int pin_i = 0; pin_i < TouchesCount; pin_i++) {
    int pin = analogRead( Touches[pin_i] );
    int v = hzmax[pin_i]->update( pin );
  }
}

void hzmax_loop() {
  // development: one value pin, to play with resistor value.
  // 10Mohm. 450|1000 750 not quite touching. beta of 2 is plenty
  const int Pin = A0;
  static HzMax hz(1000 / 60, 2);

  int pin = analogRead(Pin);
  int v = hz.update( pin );

  print(v); print(" ");
  print("0 1000");
  println();
}
