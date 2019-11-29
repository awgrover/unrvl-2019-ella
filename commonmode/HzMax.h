#pragma once
#include "ExponentialSmooth.h"
class HzMax {
  /*
  Keep a max over a short period.
  I.e. long cap touch gets 60hz, try to get max
  */

  private:
  int window_ms; // window size
  ExponentialSmooth *smooth;
  int current_max;
  unsigned long window_start;

  public:
  HzMax(int window_ms, int beta) 
    : window_ms(window_ms),
      window_start(0) // too force update soonest
    {
    this->smooth = new ExponentialSmooth(beta); 
    } // exp smooth of max

  int update(int new_value) {
    // restart window?
    if (millis() - window_start > window_ms) {
      window_start = millis();
      smooth->average( current_max ); // update average
      current_max = 0;
    }

    // find max in this window
    if (new_value > current_max) {
      current_max = new_value;
    }

    return smooth->value();
  }

  int value() { return smooth->value(); }
};
