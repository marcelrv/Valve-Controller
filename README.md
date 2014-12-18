Valve-Controller
================

Arduino Valve Controller

This program controls up to 4 valves using the  Adafruit motor shield and
 It is connected to the internet via MQTT.
 
 Several tasks are done by the program
 - get temperature from onewire DS1820 temperature sensors
 - Defines treshold points and switches motorised valves on temperature tresholds
 - get time from the internet
 - publishes updates via MQTT
 - subscribes to the topic "cmd" and takes the messages as commands
 - Stores the configuration in the EEPROM and allows to erease it and populate with default values
