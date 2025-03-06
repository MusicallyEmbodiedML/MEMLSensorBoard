#ifndef _SERIALCOMMS_H
#define _SERIALCOMMS_H

#include "SLIP.h"
// #include "pico/hardware.h"


struct serialMessage {
    uint8_t msg;
    float value;
};

inline void __not_in_flash_func(sendOverSerial) (uint8_t msgType, float value) {
  static uint8_t __not_in_flash("serialdata") slipBuffer[64];
  serialMessage msg {msgType, value};
  unsigned int slipSize = SLIP::encode(reinterpret_cast<uint8_t*>(&msg), sizeof(serialMessage), &slipBuffer[0]);
  int res = Serial2.write(reinterpret_cast<uint8_t*>(&slipBuffer), slipSize);
}

#endif