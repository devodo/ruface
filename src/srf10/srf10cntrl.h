/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * SRF10 Controller
 */

#ifndef SRF10CNTRL_H
#define SRF10CNTRL_H

#include "../lib/datatypes.h"
#include "srf10.h"

#define SRF10_DEVICES 5

// srf10 i2c states
enum SRF10State {RANGE, POLL, REQUEST, READ};

/******************************************************
 * class SRF10Controller
 * 
 * Coordinates the firing of an SRF10 by managing 
 * the i2c state and timing of an SRF10 instance.
 *
 *****************************************************/
class SRF10Controller {
 public:
  // sets minimum delay between firing
  static void setMinDelay(const tU32 &delay) { minFireDelay = delay; };
  static const tU32 &getMinDelay() { return minFireDelay; };
  // sets minimum difference of the firing differences
  static void setMinDelayDiff(const tU32 &delayDiff) {minDelayDiff = delayDiff;};
  static const tU32 &getMinDelayDiff() { return minDelayDiff; };
  // set flag to use dynamic difference timing algorithm
  static void setDynamicTiming(const bool &use) { useDynamicTiming = use; };
  static bool usingDynamicTiming() { return useDynamicTiming; };
  // notifies all controller instances that the timer will be reset
  static void notifyTimerReset();
  // resets all controller instances' state and timing values
  static void resetAll();

  SRF10Controller(SRF10 *s, tU8 n); // constructor
  SRF10 *getSRF10() { return srf10; }; // returns the SRF10 instance
  void onBusSuccess(); // callback function when i2c command is successful
  const SRF10State &getState() const { return state; }; // returns the state
  void doNextAction(); // execute next i2c command
  tU32 calculateFireTime(); // returns the desired delay before next firing
  char getName() const { return name; };
  tU8 getIndex() const { return index; }; // for statically indexing this
  // resets state and timing values
  void reset();

 private:
  static SRF10Controller *controller[]; // pointer to all controllers
  static tU8 count; // the controller instance count
  static tU32 minFireDelay; // the minimum time between firing
  static tU32 minDelayDiff; // the min firing difference between delays
  static bool useDynamicTiming; // use the timing algorithm flag
  static tU32 globalTime; // a record of the actual time, for analysis
  static tU32 offsetTime; // for maintaining current time on timer resets
  static tU32 lastFireTime[]; // last firing times of all controllers
  static tU8 lastFirerIndex; // index to last controller that fired

  // notifies all controllers that a controller has been scheduled to fire
  static void notifyFiring(const tU8 &firerIndex, const tU32 &fireTime);
  static void trimOffsetTime();

  SRF10 *srf10; // the SRF10 instance that this controls
  SRF10State state; // the i2c state
  tU8 index; // for indexing the static controller pointer
  tU8 name; // identifier
  // The time differences from last fire time of all other SRF10s
  tU32 lastDelay[SRF10_DEVICES];
  
};
#endif
