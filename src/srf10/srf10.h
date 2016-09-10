/* David Naylor    Ultrasonic Ranging for Model Helicopters
 *
 * SRF10
 */

#ifndef SRF10_H
#define SRF10_H

#include "../lib/datatypes.h"

class SRF10 {
 public:
  SRF10(tU16 addr);
  void doRangeMs();
  void doRangeCm();
  void doRangeInch();
  void doRequestRangeData();
  void doReadRangeData();
  void doPoll();
  bool hasReading();
  void doUpdateRangeData();
  tU16 getRangeData() { newData = false; return rangeData; };
  void changeAddressS1();
  void changeAddressS2();
  void changeAddressS3();
  void setNewAddress(tU16 newAddress);
  tU16 getAddress() const { return address; };
  void setMaxRange(tU8 rangeValue);
  void setGain(tU8 gainValue);
  tU8 getMaxRange() { return rangeSetting; };
  tU8 getGain() { return gainSetting; };
  bool hasUpdate() { return newData; };

 private:
  tU16 address;
  tU16 rangeData;
  bool newData;
  tU8 rangeSetting;
  tU8 gainSetting;
};

#endif
