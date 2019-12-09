#pragma once

template <typename T1, typename T2>
class CrossOverDetect {
/* Detects when 2 values cross (by at least some delta), and tracks the state
  e.g. 2 cap touch that are smoothed
  Given 2 objects that have .value, provides
  .state = -1, 0, 1 for current state (0 is indeterminate)
  # .changed = 0|1, will only provide 1 once to indicate a change
*/

  private:
  short _state;
  boolean _changed;

  public:
  // leave 'em public for debugging
  T1 &v1; // usually the fast
  T2 &v2;
  int delta;

  // starts at state == 0, delta == 1
  CrossOverDetect(T1 *v1, T2 *v2) : delta(1), v1(*v1), v2(*v2), _state(0), _changed(0) {};
  CrossOverDetect(int delta, T1 *v1, T2 *v2) : delta(delta), v1(*v1), v2(*v2), _state(0), _changed(0) {};
  // If you want to start in a specific state
  CrossOverDetect(T1 *v1, T2 *v2, int start_state) : delta(1), v1(*v1), v2(*v2), _state(start_state), _changed(0) {};
  CrossOverDetect(int delta, T1 *v1, T2 *v2, int start_state) : delta(delta), v1(*v1), v2(*v2), _state(start_state), _changed(0) {};

  int state() {
    // HAVE to call this to calculate if state changed
    // -1 (on), 0 (?), +1 (off)
    if ( _state != -1 && v2.value() - v1.value() >= delta ) { _state = -1; _changed=1; }
    else if ( _state != 1 && v1.value() - v2.value() >= delta ) { _state = 1; _changed=1; }
    return _state;
    }

  int current_delta() { return v1.value() - v2.value(); } // mostly for debugging

  boolean on() {
    // convenience if state == 1, i.e. if v1 > v2
    return state() == 1;
    }

  boolean off() {
    // convenience if state == -1, i.e. if v1 < v2
    // NB: remember we can be in state 0: indeterminate
    return state() == -1;
    }

  boolean d_on() {
    // true only once when change to on
    return state() == 1 && changed(); // relies on short-circuit
  }
  boolean d_off() {
    // true only once when change to off
    return state() == -1 && changed(); // relies on short-circuit
  }

  boolean changed() {
    boolean temp = _changed;
    _changed = 0; // always reset on query
    return temp; // but return what it was
    }
};
