#include "protocol2.h"



void RS485BUS::printPackageBuffer()
{
  for(int i = 0; i < packageBufferLength; i++)
  {
    Serial.print(packageBuffer[i], HEX);
    Serial.print(", ");
  }
  Serial.println("");
}



bool RS485BUS::ping(byte address)
{
  byte sendArray[1] = {0x01};
  if(sendPacket(address, sendArray,1))
  {
    return packageBuffer[4] == address;
  }
  else
    return false;
}

void RS485BUS::setLED(byte address, bool led)
{
  unsigned int tableAddress = 0x0041;
  byte sendArray[4];
  sendArray[0] = 0x03;
  sendArray[1] = tableAddress & 0x00FF;
  sendArray[2] = tableAddress >> 8;
  sendArray[3] = led;
  sendPacket(address, sendArray, 4);
}

void RS485BUS::toggleTorque(byte address, bool torque)
{
  unsigned int tableAddress = 64;
  byte sendArray[4];
  sendArray[0] = 0x03; //write
  sendArray[1] = tableAddress & 0x00FF;
  sendArray[2] = tableAddress >> 8;
  sendArray[3] = torque;
  sendPacket(address, sendArray, 4);
}

void RS485BUS::setPGain(byte address, unsigned int gain)
{
  unsigned int tableAddress = 84;
  byte sendArray[5];
  sendArray[0] = 0x03;
  sendArray[1] = tableAddress & 0x00FF;
  sendArray[2] = tableAddress >> 8;
  sendArray[3] = gain & 0xFF;
  sendArray[4] = gain >> 8;
  sendPacket(address, sendArray, 5);
}

void RS485BUS::reboot(byte address)
{
  byte sendArray[1];
  sendArray[0] = 0x08; //reboot
  sendPacket(address, sendArray, 1);
}

void RS485BUS::factoryReset(byte address)
{
  byte sendArray[2];
  sendArray[0] = 0x06; //reset
  sendArray[1] = 0x02; //do not reset id,baudrate
  sendPacket(address, sendArray, 2);
}


byte RS485BUS::getTemp(byte address)
{
  return readControlTable(address, 146, 1);
}

RS485BUS::RS485BUS(int _RTS_pin, long baudrate)
{
  RTS_pin = _RTS_pin;
  pinMode(RTS_pin, OUTPUT);
  Serial1.begin(baudrate);
}

long RS485BUS::readControlTable(byte address, unsigned int tableAddress, unsigned int tableSize)
{
  byte sendArray[5];
  sendArray[0] = 0x02;
  sendArray[1] = tableAddress & 0x00FF;
  sendArray[2] = tableAddress >> 8;
  sendArray[3] = tableSize & 0x00FF;
  sendArray[4] = tableSize >> 8;

  if(sendPacket(address, sendArray, 5))
  {
    byte readData[tableSize];

    for (int i = 0; i < tableSize; i++) //get data from buffer
    {
      readData[i] = packageBuffer[i + 9];
    }

    unsigned long returnValue = 0;
    for (int i = 0; i < tableSize; i++) //arrange data in long variable
    {
      /*
      Serial.print("Returnvalue: ");
      Serial.println(returnValue);
      Serial.print("(readData[i] << (8 * i): ");
      Serial.println((readData[i] << (8 * i)));
      */
      returnValue = returnValue | (readData[i] << (8 * i));

    }
    return returnValue;
  }
  else
  {
    Serial.print("Failed to read from address: ");
    Serial.println(address);
    return -1;
  }
}

bool RS485BUS::sendPacket(byte address, byte* params, byte param_length)
{
  digitalWrite(RTS_pin, HIGH);
  unsigned int arrayLength = param_length + 7;
  byte SendingArray[arrayLength];
  SendingArray[0] = 0xFF;
  SendingArray[1] = 0xFF;
  SendingArray[2] = 0xFD;
  SendingArray[3] = 0x00;
  SendingArray[4] = address;
  unsigned int MesLength = param_length + 2;
  SendingArray[5] = MesLength & 0x00FF;
  SendingArray[6] = MesLength >> 8;

  for (int i = 7; i < arrayLength; i++) { //copy params into the sending array
    SendingArray[i] = params[i - 7];
  }

  //calculate CRC
  unsigned int crc_int = update_crc(0, SendingArray, arrayLength);

  //Write message to serial buffer:
  for (int i = 0; i < arrayLength; i++) {
    Serial1.write(SendingArray[i]);
  }
  //Write crc to buffer:
  Serial1.write(crc_int & 0x00FF);
  Serial1.write((crc_int >> 8) & 0x00FF);

  //Wait till buffer is emptied:
  Serial1.flush();
  //done writing
  digitalWrite(RTS_pin, LOW);


  //Now the return packet must be read
  //first 7 bytes of packet, allows to get the length of the rest of the packet
  byte incomingBuffer[7];
  for (int i = 0; i < 7; i++) {
    unsigned long startTime = millis();
    while (!Serial1.available()) {
      if (millis() > startTime + 2) {  //time-out after 2ms
        //Serial.println("time-out on packet recieve");
        return false;
      }
    }
    incomingBuffer[i] = Serial1.read();
  }

  //now the first 7 bytes are hopefulle received, check this assumption:
  if (incomingBuffer[0] == 0xFF && incomingBuffer[1] == 0xFF && incomingBuffer[2] == 0xFD) {
    //get length from the length fields:
    unsigned int paramLength = 0;
    paramLength = incomingBuffer[6] << 8;
    paramLength = paramLength | incomingBuffer[5];


    packageBufferLength = paramLength + 7;
    memcpy(packageBuffer, incomingBuffer, 7); //move first part of msg to buffer

    //get the rest of the message
    for (int j = 7; j < packageBufferLength; j++) {
      while (!Serial1.available()) {
      }
      packageBuffer[j] = Serial1.read();
    }

    /*
    Serial.print("response, packageBuffer: ");
    for (int j = 0; j < packageBufferLength; j++) {
      Serial.print(packageBuffer[j], HEX);
      Serial.print(" ");
    }
    */

    //finally check that CRC is correct:
    unsigned int calculatedCRC = update_crc(0, packageBuffer, packageBufferLength - 2);
    if ((packageBuffer[packageBufferLength - 1] == (calculatedCRC >> 8) & 0x00FF) && (packageBuffer[packageBufferLength - 2] == (byte)calculatedCRC & 0x00FF))
      return true;
    else
    {
      Serial.println("bad CRC");
      return false;
    }
  }
  else  //bad packet received
  {
    Serial.println("Failed to read packet");
    return false;
  }
}

unsigned int RS485BUS::update_crc(unsigned int crc_accum, byte *data_blk_ptr, unsigned int data_blk_size)
{
  unsigned int i, j;
  unsigned int crc_table[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
  };

  for (j = 0; j < data_blk_size; j++)
  {
    i = ((unsigned int)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
    crc_accum = (crc_accum << 8) ^ crc_table[i];
  }

  return crc_accum;
}
