/*
 Temperature Controlled Valve
 Copyright (c) 2014 Marcel Verpaalen.  All right reserved.
 
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

DS1820 block for identifying the sensors and reading temperatures from one of the sensors
*/

#include <OneWire.h>
#include <EEPROMex.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(44);  // on pin 10 (a 4.7K resistor is necessary)

#define DEBUGGING false
//#define EEPROM_ADDRESS_NUM_SENSORS 10
//#define EEPROM_ADDRESS_SENSOR_ID EEPROM_ADDRESS_NUM_SENSORS + 1
//#define EEPROM_MAX_DEVICES 10
//

byte identifySensors(boolean writeToEEPROM) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  byte numSensors = 0;

  Serial.println(F("Identify Temperature sensors"));
  while (ds.search(addr)) {



    Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }


    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return 0;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (addr[0]) {
      case 0x10:
        Serial.println("  Chip = DS18S20");  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
        Serial.println("  Chip = DS18B20");
        type_s = 0;
        break;
      case 0x22:
        Serial.println("  Chip = DS1822");
        type_s = 0;
        break;
      default:
        Serial.println("Device is not a DS18x20 family device.");
        return 0;
    }

//todo: move to eeprom module
    if (writeToEEPROM && numSensors < MAX_TEMP_SENSORS) {
      Serial.println (EEPROM.updateBlock<byte>(address_SensorsIDs + numSensors * 8, addr, 8));
 //     for (int j = 0; j < 8; j++) sensorIDs[i][numSensors] = addr[i];
 //     sensorIDs[numSensors][i] = addr;
    }

    numSensors += 1;

  }
  Serial.println("No more addresses.");
  Serial.print("Found ");
  Serial.print(numSensors );
  Serial.println(" sensors" );
  ds.reset_search();
  delay(250);

//todo: move to eeprom module
  if (numSensors < MAX_TEMP_SENSORS) {
    EEPROM.updateByte(address_numSensors, numSensors);
  } else EEPROM.updateByte(address_numSensors, MAX_TEMP_SENSORS);

  return numSensors;
}


float temperature(byte SensorID) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;

//todo: move to eeprom module
  EEPROM.readBlock<byte>(address_SensorsIDs + SensorID * 8, addr, 8);


  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  if (DEBUGGING) {
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
  }
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  if (DEBUGGING) {
    for ( i = 0; i < 9; i++) {           // we print 9 bytes
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  if (DEBUGGING) {
    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.println(" Celsius, ");
  }
  return celsius;
}


