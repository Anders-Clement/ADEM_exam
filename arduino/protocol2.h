
#include "Arduino.h"

#ifndef PROTOCOL_2
#define PROTOCOL_2


class RS485BUS{
public:
  RS485BUS(int _RTS_pin, long baudrate);
  bool ping(byte address);
  void toggleTorque(byte address, bool torque);
  void setPGain(byte address, unsigned int gain);
  void reboot(byte address);
  void factoryReset(byte address);
  byte getTemp(byte address);
  void setLED(byte address, bool led);
  long readControlTable(byte address, unsigned int tableAddress, unsigned int tableSize);
  void printPackageBuffer();

private:
  bool sendPacket(byte address, byte* params, byte param_length);

  unsigned int update_crc(unsigned int crc_accum, byte *data_blk_ptr, unsigned int data_blk_size);

  int RTS_pin;
  byte packageBuffer[255];
  byte packageBufferLength;

};


#endif
