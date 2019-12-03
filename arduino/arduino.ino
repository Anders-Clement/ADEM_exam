#include "protocol2.h"

#define DYNAMIXEL_SERVO_BAUDRATE 1000000
#define RTS 9
byte* motors;
byte numOfMotors;
unsigned long runTime;
RS485BUS* bus_ptr;

void setup()
{
  Serial.begin(57600);
  bus_ptr = new RS485BUS(RTS, DYNAMIXEL_SERVO_BAUDRATE);
  Serial.println("Setup");
  delay(1000);

  findMotors(true);
  testGetTemp();
  testPGainSetting(0);

  /*
  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));

  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->setPGain(motors[i], 1);

  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));

  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->factoryReset(motors[i]);

  delay(1000);
  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));

  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->reboot(motors[i]);

  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));

  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->setPGain(motors[i], 1);

  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));


  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->getTemp(motors[i]));

  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 65, 1));

  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->setLED(motors[i], 1);

  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 65, 1));
    */

    runTime = 0;
}

void loop()
{
  unsigned long now = millis();
  if(now > runTime + 100) //run every 0.1 seconds
  {
    runTime += 100;
    //check temperatures:
    for(int i = 0; i < numOfMotors; i++)
    {
      if( bus_ptr->getTemp(motors[i]) > 70) //max temp of 70
        bus_ptr->toggleTorque(motors[i],false); //disable torque if too hot
    }
  }
}

void findMotors(bool addMotors)
{
  byte numMotors = 0;
  for(int i = 0; i < 253; i++)
  {
    if(bus_ptr->ping(i))
    {
      Serial.print("Response from id: ");
      Serial.println(i);
      numMotors++;
    }
  }
  Serial.println("");
  if(addMotors)
  {
    delete motors;
    motors = new byte[numMotors];
    numOfMotors = numMotors;

    int j = 0;
    for(int i = 0; i < 253; i++)
    {
      if(bus_ptr->ping(i))
      {
        motors[j] = i;
        Serial.print("Added motor at id: ");
        Serial.print(i);
        Serial.print(", as motor number: ");
        Serial.println(j);

        j++;
      }
    }
  }
}

void testPGainSetting(int gain)
{
  Serial.println("Previous P gains:");
  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));

  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->setPGain(motors[i], gain);

  Serial.println("New P gains:");
  for(int i = 0; i < numOfMotors; i++)
    Serial.println(bus_ptr->readControlTable(motors[i], 84, 2));
}

void factoryResetAll()
{
  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->factoryReset(motors[i]);
}

void rebootAll()
{
  for(int i = 0; i < numOfMotors; i++)
    bus_ptr->reboot(motors[i]);
}

void testGetTemp()
{
  for(int i = 0; i < numOfMotors; i++)
  {
    Serial.print("Temperature for motor ");
    Serial.print(motors[i]);
    Serial.print(": ");
    Serial.println(bus_ptr->getTemp(motors[i]));
  }
}
