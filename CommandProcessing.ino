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
 
 Command processing
 See header.h for more details
 */
#include <avr/pgmspace.h>
prog_char helptext_1[]  PROGMEM = "***Command Options***";
prog_char helptext_2[]  PROGMEM = "Format 'command valve:value' e.g. T1:65 set temperature for valve 1 to 65";
prog_char helptext_3[]  PROGMEM = "H help";
prog_char helptext_4[]  PROGMEM = "V Valve Control O = open C=closed";
prog_char helptext_5[]  PROGMEM = "T Temperature setting per valve";
prog_char helptext_6[]  PROGMEM = "M Mode setting per valve";
prog_char helptext_7[]  PROGMEM = "L Linked Sensor per valve";
prog_char helptext_8[]  PROGMEM = "D Drift allowed per valve";
prog_char helptext_9[]  PROGMEM = "N Name of valve";
prog_char helptext_10[] PROGMEM = "S Sensor name";
prog_char helptext_11[] PROGMEM = "Q MQTT setting S:[server] P:[port] N:[Name] U:Password";
prog_char helptext_12[] PROGMEM = "R Reset settings";
prog_char helptext_13[] PROGMEM = "P Print settings";
prog_char helptext_14[] PROGMEM = "W Write values to EEPROM";
prog_char helptext_15[] PROGMEM = "D Detect new sensors";
prog_char helptext_16[] PROGMEM = "U Print Update";
prog_char helptext_17[] PROGMEM = "E Erease EEPROM and restore default";

PROGMEM const char *help_table[] = 	   // change "string_table" name to suit
{ 
  helptext_1,helptext_2,helptext_3,helptext_4,helptext_5,helptext_6,helptext_7,helptext_8,helptext_9,helptext_10,helptext_11,helptext_12,helptext_13,helptext_14,helptext_15, helptext_16,helptext_17};

void helptext() {
  for (int i = 0; i < 16; i++){
    strcpy_P(msgBuffer, (char*)pgm_read_word(&(help_table[i]))); // Necessary casts and dereferencing, just copy. 
    statusMessage  (msgBuffer);
  }
}

void ProcessCommand (char* commandString) {

  switch (commandString[0]) {
  case 'h':
  case 'H':
    for (int i = 0; i < 16; i++){
      strcpy_P(msgBuffer, (char*)pgm_read_word(&(help_table[i]))); // Necessary casts and dereferencing, just copy. 
      statusMessage  (msgBuffer);
    }
    break;
  case 'e':
  case 'E':
    ereaseEEPROM();
    softReset();
    break;
  case 'r':
  case 'R':
    softReset();
    break;
  case 'p':
  case 'P':
    //readEEpromValues();
    publishStatus();
    publishEEpromValues();
    break;

  case 'u':
  case 'U':
    publishStatus();
    break;

  case 'w':
  case 'W':
    statusMessage  ("Write EEPROM");
    writeEEprom() ;
    readEEpromValues();
    publishEEpromValues();
    break;

  case 'm':
  case 'M':
    //set mode
    if (getID(commandString, 0, NUMBER_MOTORS) >= 0 ) {
      Valve_Modes[getID(commandString, 0, NUMBER_MOTORS)] = (byte) getLongValue(commandString, 0, 4);
      publish_Mode(getID(commandString, 0, NUMBER_MOTORS));
    } 
    else {
      snprintf (msgBuffer, MSGSIZE, "Command M value %s invalid", commandString + 1);
      statusMessage  (msgBuffer);
    }
    break;

  case 't':
  case 'T':
    //set temp
    if (getID(commandString, 0, NUMBER_MOTORS) >= 0 ) {
      Valve_Switch_Temperature[getID(commandString, 0, NUMBER_MOTORS)] =  getFloatValue(commandString, -200, 200);
      publish_Switch_Temperature(getID(commandString, 0, NUMBER_MOTORS));
    } 
    else {
      snprintf (msgBuffer, MSGSIZE, "Command T value %s invalid", commandString + 1);
      statusMessage  (msgBuffer);
    }
    break;

  case 'l':
  case 'L':
    //set linked sensor
    if (getID(commandString, 0, NUMBER_MOTORS) >= 0 ) {
      Valve_Linked_Sensor[getID(commandString, 0, NUMBER_MOTORS)] = (byte) getLongValue(commandString, 1, MAX_TEMP_SENSORS);
      publish_Linked_Sensor(getID(commandString, 0, NUMBER_MOTORS));
    } 
    else {
      snprintf (msgBuffer, MSGSIZE, "Command L value %s invalid", commandString + 1);
      statusMessage  (msgBuffer);
    }
    break;

  case 'd':
  case 'D':
    //set drift
    if (getID(commandString, 0, NUMBER_MOTORS) >= 0 ) {
      Valve_Drift[getID(commandString, 0, NUMBER_MOTORS)] = getFloatValue(commandString, -100, 100);
      publish_Drift(getID(commandString, 0, NUMBER_MOTORS));
    } 
    else {
      snprintf (msgBuffer, MSGSIZE, "Command L value %s invalid", commandString + 1);
      statusMessage  (msgBuffer);
    }
    break;

  case 'n':
  case 'N':
    //set name
    if (getID(commandString, 0, NUMBER_MOTORS) >= 0 ) {
      getStringValue(commandString, valveDescriptions[getID(commandString, 0, NUMBER_MOTORS)], DESCRIPTIONS_LENGHT) ;
      publish_Valve_Name (getID(commandString, 0, NUMBER_MOTORS));
    } 
    else {
      snprintf (msgBuffer, MSGSIZE, "Command N value %s invalid", commandString + 1);
      statusMessage  (msgBuffer);
    }
    break;
  case 's':
  case 'S':
    //set temp sensor name
    if (getID(commandString, 0, MAX_TEMP_SENSORS) >= 0 ) {
      getStringValue(commandString, sensorDescriptions[getID(commandString, 0, MAX_TEMP_SENSORS)], DESCRIPTIONS_LENGHT) ;
      publish_Sensor_Name(getID(commandString, 0, MAX_TEMP_SENSORS));
    } 
    else {
      snprintf (msgBuffer, MSGSIZE, "Command S value %s invalid", commandString + 1);
      statusMessage  (msgBuffer);
    }
    break;
  case 'q':
  case 'Q':
    //set name
    switch (commandString[1]) {
    case 's':
    case 'S':
      getStringValue(commandString, serverName, MAX_SERVERNAME_LENGHT) ;
      break;
    case 'p':
    case 'P':
      if (getLongValue(commandString, 0, 65535) > 0) mqttPort = getLongValue(commandString, 0, 65535);
      break;
    case 'n':
    case 'N':
      getStringValue(commandString, userName, MAX_USERNAMEPASS_LENGHT) ;
      break;
    case 'u':
    case 'U':
      getStringValue(commandString, userPass, MAX_USERNAMEPASS_LENGHT) ;
      break;

    default:
      snprintf (msgBuffer, MSGSIZE, "MQTT Setting %s not recognised", commandString);
      statusMessage  (msgBuffer);
    }
   publish_Server();

    break;
  case 'v':
  case 'V':
    //Direct valve control
    if (getID(commandString, 0, NUMBER_MOTORS) >= 0 ) {
      switch (commandString[3]) {
      case 'o':
      case 'O':
        OpenValve(getID(commandString, 0, NUMBER_MOTORS));
        break;
      case 'c':
      case 'C':
        CloseValve(getID(commandString, 0, NUMBER_MOTORS));     
        break;
      default:
        snprintf (msgBuffer, MSGSIZE, "Valve Setting %s not recognised", commandString);
        statusMessage  (msgBuffer);
      }
    }
    break;

  default:
    snprintf (msgBuffer, MSGSIZE, "Command %s not recognised", commandString);
    statusMessage  (msgBuffer);

  }

}

int getID (char* commandString, int rangeLow, int rangeUp) {
  int valve = rangeLow - 1 ;
  if (sscanf(commandString + 1, "%d" , &valve) > 0 ) {
    if (valve > rangeUp) valve = rangeLow - 1 ;
  }
  return valve;
}

int getStringValue (char* commandString, char* output, int maxlenght) {
  strncpy(output, strchr(commandString, ':') + 1, maxlenght);
  output[maxlenght] = '\0';
}

long getLongValue (char* commandString, long rangeLow, long rangeUp) {
  long longValue = rangeLow - 1;
  if (sscanf(commandString+3, "%ld"  , &longValue) > 0  ) {
    if (longValue > rangeUp) longValue = rangeLow - 1 ;
  }
  return longValue;
}

float getFloatValue (char* commandString, long rangeLow, long rangeUp) {
  float floatValue = rangeLow - 1;
  long longValue = rangeLow - 1;
  int decimal = 0;
  sscanf(commandString + 3, "%ld.%1d", &longValue,  &decimal) ;
  floatValue = (float)longValue + (float)decimal / 10;
  if (floatValue > rangeUp) floatValue = rangeLow - 1 ;
  return floatValue;
}






