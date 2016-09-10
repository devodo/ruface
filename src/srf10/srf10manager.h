#ifndef SRF10MANAGER_H
#define SRF10MANAGER_H

#include "../lib/datatypes.h"
#include "srf10.h"

#define SRF10_DEVICES 5

enum SRF10State {RANGE, POLL, REQUEST, READ};

class SRF10Manager {
 public:
  static void setMinDelay(const tU32 &delay) { minFireDelay = delay; };
  static const tU32 &getMinDelay() { return minFireDelay; };
  static void setMinDiff(const tU32 &diff) { minFireDiff = diff; };
  static const tU32 &getMinDiff() { return minFireDiff; };
  static void setDynamicTiming(const bool &use) { useDynamicTiming = use; };
  static void notifyAllTimerReset();
  static void resetAll();

  SRF10Manager(SRF10 *s, tU8 n);
  SRF10 *getSRF10() { return srf10; };
  void onBusSuccess();
  const SRF10State &getState() const { return state; };
  void doNextAction();
  tU32 calculateFireTime();
  char getName() const { return name; };
  tU8 getIndex() const { return index; };
  void notifyFiring(const tU8 &firerIndex, const tU32 &fireTime);
  void addTimeOffset(const tU32 &offset) { offsetTime += offset; };
  void reset();

 private:
  static SRF10Manager *manager[];
  static tU8 count;
  static tU32 minFireDelay;
  static tU32 minFireDiff;
  static bool useDynamicTiming;
  static tU32 globalTime;

  static void notifyAllFiring(const tU8 &firerIndex, const tU32 &fireTime);

  SRF10 *srf10;
  SRF10State state;
  tU8 index;
  tU32 offsetTime;
  tU8 name;
  tU32 lastFireTime[SRF10_DEVICES];
  tU32 lastTimeDiff[SRF10_DEVICES];
  tU8 lastFirerIndex;
};
#endif
