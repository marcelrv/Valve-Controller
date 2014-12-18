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
 
 This program controls up to 4 valves using the  Adafruit motor shield and
 It is connected to the internet via MQTT.
 
 Several tasks are done by the program
 - get temperature from onewire DS1820 temperature sensors
 - Defines treshold points and switches motorised valves on temperature tresholds
 - get time from the internet
 - publishes updates via MQTT
 - subscribes to the topic "cmd" and takes the messages as commands
 - Stores the configuration in the EEPROM and allows to erease it and populate with default values
 */

#include <EEPROMex.h>


#ifndef ValveController_h
#define ValveController_h


#define DESCRIPTIONS_LENGHT 20

#define COMMANDBUFFER_LENGHT 30
char serialCommandBuffer[COMMANDBUFFER_LENGHT];
unsigned int serialCommandBufferIndex = 0;

#define MAX_TEMP_SENSORS 10
byte numSensors = 0;
float sensorTemperature[MAX_TEMP_SENSORS];
char sensorDescriptions [MAX_TEMP_SENSORS][DESCRIPTIONS_LENGHT];
byte sensorIDs [MAX_TEMP_SENSORS][8];

#define NUMBER_MOTORS 4
byte numValves = 1;
char valveDescriptions [NUMBER_MOTORS][DESCRIPTIONS_LENGHT];
byte Valve_Modes [NUMBER_MOTORS];
byte Valve_Last_Pos [NUMBER_MOTORS];
float Valve_Switch_Temperature[NUMBER_MOTORS];
byte Valve_Linked_Sensor[NUMBER_MOTORS];
float Valve_Drift[NUMBER_MOTORS];

//EEprom memory addresses
int address_MAGIC;
int address_MQTT_Server;
int address_MQTT_Port  ;
int address_MQTT_username;
int address_MQTT_password ;
int address_numSensors     ;
int address_SensorsIDs     ;
int address_Sensors_descriptions ;
int address_numValves;
int address_Valve_descriptions ;
int address_Valve_Modes ;
int address_Valve_Last_Pos ;
int address_Valve_Switch_Temperature;
int address_Valve_Linked_Sensor;
int address_Valve_Drift;

//MQTT
#define MAX_SERVERNAME_LENGHT 30
#define MAX_USERNAMEPASS_LENGHT 15
char serverName [MAX_SERVERNAME_LENGHT] ;
char userName [MAX_USERNAMEPASS_LENGHT] ;
char userPass [MAX_USERNAMEPASS_LENGHT] ;
long  mqttPort = 1883;

//buffer used to store strings for messages
#define MSGSIZE 80
char msgBuffer[MSGSIZE];

#define TOPICSIZE 30
char TopicBuffer[TOPICSIZE];


#endif

