#include "tired_of_serial.h"
#include "PeakTrack.h"
#include "HzMax.h"
#include "OnChange.h"

// magic for array size:
template <typename T,unsigned S> inline constexpr unsigned arraysize(const T (&v)[S]) { return S; };

int Touches[] = { A0, A1, A2, A3, A4 }; // touch pins to scan
constexpr int TouchesCount = arraysize(Touches);
const char TouchLetter = '0'; // A0='0', A1='1', etc. Sent to serial port
const int Common = A5; // un attached reference antenna
HzMax *hzmax[ TouchesCount ];
OnChange *on_change[ TouchesCount ];
// Our long-lead touch wire acts like an antenna and picks up the 60Hz power
// So, the period is 1/60 *1000ms * 1.1fuzz
const int HzPeriod = 1/60.0 * 1000 * 1.1; //rounded is fine. make sure we catch at least 1 peak
const int HzMaxBeta = 3; // we don't seem to need much averaging
// "commonmode" serves as a reference base value
HzMax commonmode( HzPeriod, 20);
const float commonmode_ratio = 1.8; // the commonmode antenna is "smaller" than the real ones, about 30%
int on_threshold; // dynamically from commonmode
const float on_threshold_ratio = 0.60; // of the TouchedValue, to count as touched. 1/2 isn't enough. .7 is unstable
const int TouchedValue = 1000; // empirically

void setup() {
  Serial.begin(115200);
  hzmax_setup();
}

void hzmax_setup() {
  for (int pin_i=0; pin_i<TouchesCount; pin_i++) {
    hzmax[pin_i] = new HzMax( HzPeriod, HzMaxBeta );
    on_change[pin_i] = new OnChange();
  }
}

void loop() {
  //plot_loop();
  plot_on_loop();
  //touched_loop();
}

void touched_loop() {
  read_pins();
  for (int pin_i=0; pin_i<TouchesCount; pin_i++) {
    boolean on = hzmax[pin_i]->value() > on_threshold;
    if ( on_change[pin_i]->changed(on) ) {
      print( (char) (TouchLetter + pin_i) );print(on ? "+" : "-");println();
      }
  }
}

void plot_on_loop() {
  read_pins();
  for (int pin_i=0; pin_i<TouchesCount; pin_i++) {
  //for (int pin_i=2; pin_i<3; pin_i++) {
    boolean on = hzmax[pin_i]->value() > on_threshold;
    //print( on_threshold - hzmax[pin_i]->value() );print(" ");
    print( on * 50 + pin_i * 2 );print(" "); // 0|10 with offset so we can see them
  }
  print("-2 ");print( 50 + TouchesCount*2);print(" ");
  //print("1026 ");
  //print(on_threshold);print(" ");
  println();
}

void plot_loop() {
  read_pins();
  print(commonmode.value());print(" ");
  for (int pin_i=0; pin_i<TouchesCount; pin_i++) {
    print(hzmax[pin_i]->value());
    print(" ");
  }
  print("1026 ");
  print(on_threshold);print(" ");
  println();
}

void timeread_loop() {
  unsigned long top = millis();
  for(int i=0;i<1000;i++) {
    read_pins();
    }
  println((millis()-top)/1000.0);
  }

void read_pins() {
  // read 'em all
  // Takes about 0.58ms to read 5! 0.10ms each!
  // And about .8ms +commonmode+calculation.
  // 60Hz is about 16ms. so we should be reasonably ok

  // adjust on_threshold based on commonmode
  int common_value = commonmode.update( commonmode_ratio * analogRead(Common) );
  on_threshold = (TouchedValue + common_value ) * on_threshold_ratio;

  for (int pin_i=0; pin_i<TouchesCount; pin_i++) {
    int pin = analogRead( Touches[pin_i] );
    int v = hzmax[pin_i]->update( pin );
  }
}

void hzmax_loop() {
  // 10Mohm. 450|1000 750 not quite touching. beta of 2 is plenty
  const int Pin = A0;
  static HzMax hz(1000/60, 2);

  int pin = analogRead(Pin);
  int v = hz.update( pin );

  print(v);print(" ");
  print("0 1000");
  println();
}
  

void peak_loop() {
  // using the peak/decay class
  // works ok, slow off
  // 10Mohm 100|300
  const int Pin = A0;
  static PeakTrack a0(200);
  const float CommonEx = 1.4;

  int common = analogRead(Common);
  int pin = analogRead(Pin);

  a0.update( pin-(common * CommonEx) );

  print(800);print(" ");
  //print(common);print(" ");
  //print(pin);print(" ");
  //print(pin-(common * CommonEx));print(" ");
  print(a0.value());print(" ");
  println();
}
