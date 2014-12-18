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
 
 Get the settings from EEPROM
 See header.h for more details
 
 */
 
#include <EEPROMex.h>

//read the addresses setup from the eeprom
void setupEEprom() {
  // Always get the adresses first and in the same order
  address_MAGIC        = EEPROM.getAddress(sizeof(char) * 2);
  address_MQTT_Server  = EEPROM.getAddress(sizeof(char) * MAX_SERVERNAME_LENGHT);
  address_MQTT_Port     = EEPROM.getAddress(sizeof(long));
  address_MQTT_username = EEPROM.getAddress(sizeof(char) * MAX_USERNAMEPASS_LENGHT);
  address_MQTT_password = EEPROM.getAddress(sizeof(char) * MAX_USERNAMEPASS_LENGHT);
  address_numSensors     = EEPROM.getAddress(sizeof(byte));
  address_SensorsIDs     = EEPROM.getAddress(sizeof(byte) * MAX_TEMP_SENSORS * 8);
  address_Sensors_descriptions = EEPROM.getAddress(sizeof(char) * MAX_TEMP_SENSORS * DESCRIPTIONS_LENGHT);
  address_numValves  = EEPROM.getAddress(sizeof(byte));
  address_Valve_descriptions = EEPROM.getAddress(sizeof(char) * NUMBER_MOTORS * DESCRIPTIONS_LENGHT);
  address_Valve_Modes = EEPROM.getAddress(sizeof(byte) * NUMBER_MOTORS);
  address_Valve_Last_Pos = EEPROM.getAddress(sizeof(byte) * NUMBER_MOTORS);
  address_Valve_Switch_Temperature = EEPROM.getAddress(sizeof(float) * NUMBER_MOTORS);
  address_Valve_Linked_Sensor = EEPROM.getAddress(sizeof(byte) * NUMBER_MOTORS);
  address_Valve_Drift = EEPROM.getAddress(sizeof(float) * NUMBER_MOTORS);

  Serial.println(F("Setup EEPROM Memory"));
  Serial.print( EEPROM.getAddress(sizeof(byte)));
  Serial.println(F(" EEprom memory bytes used"));

  //check if the magic character are there, if not, initialize with default values
  if ((char)EEPROM.readByte (address_MAGIC) != 'M' || (char)EEPROM.readByte (address_MAGIC + 1) != 'V' ) InitializeEEprom();
  readEEpromValues();
}

void publishEEpromValues() {

  publish_Server();

  publishMQTT ("setting/sensors", (long)numSensors);
  for (byte i = 0; i < MAX_TEMP_SENSORS; i++) {
    publish_Sensor_Name(i);
    snprintf (TopicBuffer, TOPICSIZE, "sensor/%1d/serial", i);
    byte addr[8];
    EEPROM.readBlock<byte>(address_SensorsIDs + i * 8, addr, 8);
    snprintf(msgBuffer, MSGSIZE, "%02X%02X %02X%02X %02X%02X %02X%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
    publishMQTT (TopicBuffer, msgBuffer);

    //    snprintf(msgBuffer, MSGSIZE, "%02X%02X %02X%02X %02X%02X %02X%02X", sensorIDs[i][0], sensorIDs[i][1], sensorIDs[i][2], sensorIDs[i][3], sensorIDs[i][4], sensorIDs[i][5], sensorIDs[i][6], sensorIDs[i][7]);
    //    publishMQTT (TopicBuffer, msgBuffer);
  }

  publishMQTT ("setting/valves", (long)numValves);
  for (int i = 0; i < NUMBER_MOTORS; i++) {
    publish_Valve_Name(i);
    publish_Mode(i);
    publish_Switch_Temperature(i);
    publish_Drift(i);
    publish_Linked_Sensor(i);
    //writeValvePositions not in use for now
    //    snprintf (TopicBuffer, TOPICSIZE, "/valvesPos/%1d", i);
    //    publishMQTT (TopicBuffer, (long)Valve_Last_Pos[i]);
  }
}

void readEEpromValues() {
  Serial.println(F("Reading EEPROM Memory"));
  EEPROM.readBlock<char>(address_MQTT_Server, serverName, MAX_SERVERNAME_LENGHT);
  mqttPort = EEPROM.readLong  (address_MQTT_Port);
  EEPROM.readBlock<char>(address_MQTT_username, userName, MAX_USERNAMEPASS_LENGHT);
  EEPROM.readBlock<char>(address_MQTT_password, userPass, MAX_USERNAMEPASS_LENGHT);
  numSensors =   EEPROM.readByte (address_numSensors);
  for (int i = 0; i < MAX_TEMP_SENSORS; i++) {
    EEPROM.readBlock (address_Sensors_descriptions + i * DESCRIPTIONS_LENGHT, sensorDescriptions[i]);

    //TODO: Fix this.. not okay
    // EEPROM.readBlock<byte>(address_SensorsIDs + i * 8, sensorIDs[i],8);
    //    for (int j = 0; j < 8; j++) sensorIDs[j][i] = EEPROM.readByte(address_SensorsIDs + i * 8+j);
  }
  numValves = EEPROM.readByte (address_numValves);
  for (int i = 0; i < NUMBER_MOTORS; i++) {
    EEPROM.readBlock (address_Valve_descriptions + i * DESCRIPTIONS_LENGHT, valveDescriptions[i]);
    Valve_Modes [i] = EEPROM.readByte (address_Valve_Modes + i);
    Valve_Last_Pos [i] = EEPROM.readByte (address_Valve_Last_Pos + i);
    Valve_Switch_Temperature[i] = EEPROM.readFloat (address_Valve_Switch_Temperature + i*sizeof(float));
    Valve_Linked_Sensor[i] = EEPROM.readByte (address_Valve_Linked_Sensor + i);
    Valve_Drift[i] = EEPROM.readFloat (address_Valve_Drift + i*sizeof(float));
  }

}

void writeEEprom() {
  Serial.println(F("Storing data to EEPROM Memory"));

  //mqtt data store
  EEPROM.updateBlock<char>(address_MQTT_Server, serverName, MAX_SERVERNAME_LENGHT);
  EEPROM.updateBlock<char>(address_MQTT_username, userName, MAX_USERNAMEPASS_LENGHT);
  EEPROM.updateBlock<char>(address_MQTT_password, userPass, MAX_USERNAMEPASS_LENGHT);
  EEPROM.updateInt (address_MQTT_Port, mqttPort);

  //sensors data store
  EEPROM.updateByte (address_numSensors, numSensors);
  for (int i = 0; i < MAX_TEMP_SENSORS; i++) {
    EEPROM.updateBlock (address_Sensors_descriptions + i * DESCRIPTIONS_LENGHT, sensorDescriptions[i]);
    //  EEPROM.updateBlock<byte>(address_SensorsIDs + i * 8, sensorIDs[i],8);
  }

  //valves setup data store
  EEPROM.updateByte (address_numValves, numValves);
  for (int i = 0; i < NUMBER_MOTORS; i++) {
    EEPROM.updateBlock (address_Valve_descriptions + i * DESCRIPTIONS_LENGHT, valveDescriptions[i]);
    EEPROM.updateByte (address_Valve_Modes + i, Valve_Modes [i]);
    EEPROM.updateByte (address_Valve_Last_Pos + i,  Valve_Last_Pos [i]);
    EEPROM.updateFloat (address_Valve_Switch_Temperature + i*4,  Valve_Switch_Temperature [i]);
    EEPROM.updateByte (address_Valve_Linked_Sensor + i,  Valve_Linked_Sensor [i]);
    EEPROM.updateFloat (address_Valve_Drift + i*4, Valve_Drift[i]);
  }
}

void InitializeEEprom() {
  Serial.println(F("Initialize EEPROM Values"));

  //MQTT defaults
  snprintf (serverName, MAX_SERVERNAME_LENGHT, "m21.cloudmqtt.com");
  mqttPort = 16929L;
  snprintf (userName, MAX_SERVERNAME_LENGHT, "valveUSER");
  snprintf (userPass, MAX_SERVERNAME_LENGHT, "valvePWD");

  //Temperature Sensor defaults
  numSensors = 0;
  numSensors = identifySensors(true);
  for (int i = 0; i < MAX_TEMP_SENSORS; i++) {
    snprintf (sensorDescriptions[i], DESCRIPTIONS_LENGHT, "Sensor %d", i);
    for (int j = 0; j < 8; j++) sensorIDs [i][j] = 0;
  }

  //Valves defaults
  numValves = 1;
  for (int i = 0; i < NUMBER_MOTORS; i++) {
    snprintf (valveDescriptions[i], DESCRIPTIONS_LENGHT, "Valve %d", i);
    Valve_Modes [i] = 0;
    Valve_Last_Pos [i] = 255;
    Valve_Switch_Temperature[i] = 80;
    Valve_Linked_Sensor[i] = i;
    Valve_Drift[i] = 5;
  }

  writeEEprom();
  EEPROM.updateByte (address_MAGIC, (byte)('M'));
  EEPROM.updateByte (address_MAGIC + 1 , (byte)('V') );

}

void writeValvePosition(int ValveNumber, uint8_t MotorPosition) {
  EEPROM.updateByte (address_Valve_Last_Pos + ValveNumber,  MotorPosition);
}


void ereaseEEPROM() {
  for (int i = 0; i < 511; i++)  EEPROM.updateByte (i, 0);
}