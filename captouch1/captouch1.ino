/* This is a test of cap-touch for Ella's project: several objects she will coat with leaf/foil.

  Standard cap-touch does not like long leads.
  * could do "driven shield", but I don't understand it yet
  * common-mode is possible, but you need 1 for each channel (so x2)
  * "auto" exponential-crossover seems to work pretty well here!
  
  More tests:
  * test objects again with various resistor values.
  * Saw some modes where all we got was "more" signal vs less. Signal was 60hz (antenna!).
    * So, write code to do amplitude detect
    do an exp-smooth on abs(max/min). eh... 1st differential for peak detect?
    use that long smooth as the comparison base for less/more: same as cross-over detect.
    abs -> don't worry about min/max. just peak
    peak detect: another expsmooth & 1st diff (min delta)?
      or max-this-cycle till current value drops significantly below last max (so vs noise)
    exp-smooth that peak. crossover for detect
*/

#include "ExponentialSmooth.h"
#include "CrossOverDetect.h"
#include "tired_of_serial.h"

CrossOverDetect<ExponentialSmooth> cap(40, new ExponentialSmooth(50), new ExponentialSmooth(20), 1); // start at +1

const int CapScale = 400;
const int Touch = A1;

void setup() {
  Serial.begin(115200);
  Serial.println("\nStart");

  cap.v1.reset( cap.v2.reset(analogRead(Touch)) ); // initial value
}

void loop() {
    int v = analogRead(Touch);
    cap.v1.average( v );
    cap.v2.average( v );

    ////print( v );print(" ");
    print( cap.on() * CapScale );print(" ");
    print( (int) (cap.changed() * CapScale * 1.1) );print(" ");
    print( cap.v1.value() );print(" ");
    print( cap.v2.value() );print(" ");
    print( cap.v1.value() - cap.v2.value() );print(" ");
    println();

    //delay(10);

}
