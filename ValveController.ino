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
 
 See header.h for more details
 */

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Time.h>
#include <EthernetUdp.h>
#include "header.h"

#define BUFSIZE 75
char buffer[BUFSIZE];


// Update these with values suitable for your network.
byte mac[]    = {  
  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte ip[]     = { 
  192, 168, 3, 233 };

#define intervalMotor 45000L //motor switch
unsigned long lastMotorUpdate = 0;
int OpenClosed = 0;

#define intervalMeasure 10000L //temperature measurements
unsigned long lastMeasureUpdate = 0;

#define intervalStatus 30000L //mqtt server updates
unsigned long lastStatusUpdate = 0;


EthernetClient ethClient;

void setup()
{
  //setup serial port
  Serial.begin(9600);

  //read values from eeprom memory
  setupEEprom();

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // configuere with Fixed IP
    Ethernet.begin(mac, ip);

  }

  setupMQTT();
  print_IP();

  setupNTP();
  publishTimestamp();
  numSensors = identifySensors(false);
  motorsetup();
  publishEEpromValues();
}

//publishes the a message to the status topic
void statusMessage(char* msg) {
  publishMQTT("info/status", msg,true);
}

//publishes the current time
void publishTimestamp() {
  getDateTime(msgBuffer, MSGSIZE);
  publishMQTT("info/timestamp", msgBuffer,true);
}

//publishes the status
void publishStatus() {
  char Dbuffer[20];
  for (int i = 0; i < numSensors; i++) {
    dtostrf(sensorTemperature[i], 10, 2, Dbuffer);  //dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
    snprintf (msgBuffer, MSGSIZE, "Sensor %d temperature = %s Celsius", i, Dbuffer);
    Serial.println   (msgBuffer);
    snprintf (Dbuffer, 20, "sensor/%1d/value", i);
    publishMQTT(Dbuffer, sensorTemperature[i],false);
  }
  publishTimestamp();
  publishMQTT("info/memory", (long) freeRam (),true);
  for (int i = 0; i < numValves; i++){
    publish_MotorPos(i);
  publish_Mode(i);  
}
}

void updateTemperatures() {
  for (int i = 0; i <= numSensors; i++) {
    sensorTemperature[i] = temperature(i);
  }
}

//do the switch logic here
void  switchingLogicLoop() {
  for (int i = 0; i < numValves; i++) {

    //mode 0 = automatic
    if (Valve_Modes[i] == 0) {
      if ( sensorTemperature[Valve_Linked_Sensor[i]] > Valve_Switch_Temperature[i] ) ValveOpen(i);
      if ( sensorTemperature[Valve_Linked_Sensor[i]] < Valve_Switch_Temperature[i] -  Valve_Drift[i] ) ValveClose(i);
    }
    //mode 2 = temp difference
    if (Valve_Modes[i] == 2) {
      int tempDiff =  (int) abs( sensorTemperature[0] -   sensorTemperature[1]);
      if (tempDiff > 2) ValveOpen(0);
      if (tempDiff < 2) ValveClose(0);
    }
  }
}

void loop()
{
  //maintenance tasks
  MQTTLoopnConnect();
  valveLoop();
  switchingLogicLoop();
  if (serialCommandLoop()) {

    // Allocate the correct amount of memory for the payload copy
    char* cmd = (char*)malloc(strlen(serialCommandBuffer) +1);
    // Copy the payload to the new buffer
    memcpy(cmd, serialCommandBuffer, strlen(serialCommandBuffer)+1);
    ProcessCommand(cmd);
    // Free the memory
    free(cmd);

    //ProcessCommand (serialCommandBuffer);

  }

  //updates temperatures
  if ( millis() > (lastMeasureUpdate + intervalMeasure) ) {
    updateTemperatures();
    lastMeasureUpdate = millis();
  }

  if ( millis() > (lastStatusUpdate + intervalStatus) ) {
    digitalClockDisplay();
    Serial.println();
    publishStatus();
    lastStatusUpdate = millis() ;
  }
  if ( millis() > (lastMotorUpdate + intervalMotor) ) {
 //not used for now
    lastMotorUpdate = millis() ;
  }

}

void print_IP() {
  // print your local IP address:
  snprintf(msgBuffer, MSGSIZE, "IP address: %d.%d.%d.%d", Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
  statusMessage (msgBuffer);
}

// takes the data from the serial port and puts it in a buffer. Returns true if the buffer is full or return LF is detected
boolean serialCommandLoop() {
  boolean newCommandAvailable = false;
  while (Serial.available () > 0) {
    serialCommandBuffer[serialCommandBufferIndex] = Serial.read ();
    if (serialCommandBuffer[serialCommandBufferIndex] == '\n') newCommandAvailable = true; 
    else serialCommandBufferIndex++;
    if ( serialCommandBufferIndex >= COMMANDBUFFER_LENGHT ) newCommandAvailable = true;
    if (newCommandAvailable) {
      serialCommandBuffer[serialCommandBufferIndex] = 0;
      serialCommandBufferIndex = 0;
      Serial.println(serialCommandBuffer);
      return newCommandAvailable;
    }
  }
  return newCommandAvailable;
}


int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// soft reset implementation function
void softReset() {
  asm volatile ("  jmp 0");
}

