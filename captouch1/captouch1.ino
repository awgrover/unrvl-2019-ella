#include "ExponentialSmooth.h"

ExponentialSmooth fast_follower(100);
ExponentialSmooth slow_follower(200);

const int Touch = A0;
const int RefTouch = A2;
const int PosHit = 3;
const int NegHit = -PosHit;
int state = -10;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Start");
  fast_follower.reset( slow_follower.reset( analogRead(Touch) ) );
}

void loop() {
  // put your main code here, to run repeatedly:
  int v = analogRead(Touch);
  int v2 = analogRead(RefTouch);
  { // if (v - v2 > 0) {
    slow_follower.average( v );
    fast_follower.average( v );

    int delta = fast_follower.smoothed() - slow_follower.smoothed();
    
    if (state > 0 && delta >= PosHit) { state = -10; }
   else if (state < 0 && delta <= NegHit) { state = 10; }

    Serial.print("60 -4 4 ");
    //Serial.print(v); Serial.print(" "); 
    Serial.print(state);Serial.print(" ");
    Serial.print(fast_follower.smoothed());Serial.print(" "); 
    Serial.print(slow_follower.smoothed());Serial.print(" ");
    Serial.print( delta );Serial.print(" ");
    Serial.println();
  }
}
