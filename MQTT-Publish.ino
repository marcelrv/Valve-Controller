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
 
 
 MQTT connectivity module
 
 */

//MQTT Settings
#define MQTT_Prefix "/vc/"
#define MQTT_commandtopic "/vc/cmd"

#define TBUFSIZE 40

#define ClientID "arduinoClientX"

#define intervalReconnect 60000L //MQTT reconnect attempts
unsigned long lastMQTTReconnect = 0;

//byte server[] = { 192, 168, 3, 45 };
//char server[MAX_SERVERNAME_LENGHT] = "192.168.3.45";
//#define MQTT_PORT 1883

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(serverName, mqttPort, callback, ethClient);

void setupMQTT()
{
  if (mqttConnect()) {
    Serial.println(F("Setup MQTT Client"));
  } 
  else    Serial.print(F("ERROR: MQTT not available!"));

}

boolean mqttConnect () {
  boolean mqttConnected = false;
  if (millis() > lastMQTTReconnect){
    if (strlen (userName) == 0) mqttConnected = client.connect(ClientID); 
    else mqttConnected = client.connect(ClientID, userName, userPass);
    client.subscribe(MQTT_commandtopic);
    lastMQTTReconnect = millis() + intervalReconnect;
  }
  return mqttConnected;
}


// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  //convert byte to char
  payload[length] = '\0';

  Serial.print(F("Received command on "));
  Serial.print(topic);
  Serial.print(F(" : "));
  Serial.println((char*)payload);

  // Allocate the correct amount of memory for the payload copy
  char* cmd = (char*)malloc(length + 1);
  // Copy the payload to the new buffer
  memcpy(cmd, payload, length + 1);
  ProcessCommand(cmd);
  // Free the memory
  free(cmd);
}

void publishMQTT(char* topic, float publishvalue) {
  publishMQTT(topic, publishvalue, true);
}

void publishMQTT(char* topic, float publishvalue, boolean print2Serial) {
  char Dbuffer[TBUFSIZE];
  dtostrf(publishvalue, 4, 2, Dbuffer);  //4 is mininum width, 6 is precision
  publishMQTT(topic, Dbuffer, print2Serial);
}

void publishMQTT(char* topic, int publishvalue) {
  publishMQTT(topic, (long)publishvalue, true);
}

void publishMQTT(char* topic, byte publishvalue) {
  publishMQTT(topic, (long)publishvalue, true);
}

void publishMQTT(char* topic, long publishvalue) {
  publishMQTT(topic, publishvalue, true);
}

void publishMQTT(char* topic, long publishvalue, boolean print2Serial) {
  char Dbuffer[TBUFSIZE];
  snprintf (Dbuffer, TBUFSIZE, "%ld", publishvalue);
  publishMQTT(topic, Dbuffer, print2Serial);
}

void publishMQTT(char* topic, char* msg) {
  publishMQTT(topic, msg, true);
}
void publishMQTT(char* topic, char* msg, boolean print2Serial) {

  char Tbuffer[TBUFSIZE];
  snprintf (Tbuffer, TBUFSIZE, "%s%s", MQTT_Prefix, topic);
  if (client.loop()) client.publish(Tbuffer, msg);
  if (print2Serial) {
    Serial.print (Tbuffer) ;
    Serial.print (" : ") ;
    Serial.println (msg);
  }
  //incorporate small delay to avoid the subscribers to possibly miss items
  delay(100);
  MQTTLoopnConnect();
}

void MQTTLoopnConnect() {
  if (!client.loop() ) {
    Serial.print(F("Lost MQTT Connection :"));
    Serial.println(millis());
    if (mqttConnect()) {
      Serial.println(F("Reconnected"));
      delay(50);
    }
  }
  //else{Serial.println(F("MQTT Connected"));}
  //delay(50);

}


void publish_Drift(int valveNum) {
    snprintf (TopicBuffer, TOPICSIZE, "valves/%1d/drift", valveNum);
    publishMQTT (TopicBuffer, Valve_Drift[valveNum]);    
}


void publish_Switch_Temperature(int valveNum) {
   snprintf (TopicBuffer, TOPICSIZE, "valves/%1d/temp", valveNum);
    publishMQTT (TopicBuffer, Valve_Switch_Temperature[valveNum]);  
}

void publish_Mode(int valveNum) {
    snprintf (TopicBuffer, TOPICSIZE, "valves/%1d/mode", valveNum);
    publishMQTT (TopicBuffer, (long)Valve_Modes[valveNum]);
}

void publish_Valve_Name(int valveNum) {
    snprintf (TopicBuffer, TOPICSIZE, "valves/%1d/name", valveNum);
    publishMQTT (TopicBuffer, valveDescriptions[valveNum]);
    }
    void publish_Linked_Sensor(int valveNum) {
   snprintf (TopicBuffer, TOPICSIZE, "valves/%1d/sensor", valveNum);
    publishMQTT (TopicBuffer, (long)Valve_Linked_Sensor[valveNum]);
    }
    
    void publish_Sensor_Name(int valveNum) {
        snprintf (TopicBuffer, TOPICSIZE, "sensor/%1d/name", valveNum);
    publishMQTT (TopicBuffer, sensorDescriptions[valveNum]);
    }
    
 
void publish_Server(){

 publishMQTT ("setting/server", serverName);
  publishMQTT ("setting/port",  (long) mqttPort);
  publishMQTT ("setting/username", userName);
  publishMQTT ("setting/password", userPass);}
