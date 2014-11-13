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
 
 
 Time functionality
 */


int timeZone = 2;     // Central European Time
EthernetUDP udp;

time_t getNtpTime()
{
  statusMessage("Refreshing NTP time");
  unsigned long unixTime = ntpUnixTime(udp);
  return unixTime;
}

void setupNTP() {
  statusMessage("Setup NTP time");
  setSyncProvider(getNtpTime);
}


void getDate(char * timeBuffer, unsigned long lenght) {
  snprintf (timeBuffer,  lenght, "%002d-%002d-%00004d", day(), month(), year());
}

void getDateTime(char * timeBuffer, unsigned long lenght) {
  snprintf (timeBuffer,  lenght, "%002d-%002d-%00004d %002d:%002d:%002d", day(), month(), year(), hour(), minute(), second());
}

void getTime(char * timeBuffer, unsigned long lenght) {
  snprintf (timeBuffer,  lenght, "%002d:%002d:%002d", day(), month(), year(), hour(), minute(), second());
}

// digital clock display of the time
void digitalClockDisplay() {
  char dateTimeString[30];
  getDateTime(dateTimeString, 30);
  Serial.print(dateTimeString);
}


/*
 * © Francesco Potortì 2013 - GPLv3 - Revision: 1.13
 *
 * Send an NTP packet and wait for the response, return the Unix time
 *
 * To lower the memory footprint, no buffers are allocated for sending
 * and receiving the NTP packets.  Four bytes of memory are allocated
 * for transmision, the rest is random garbage collected from the data
 * memory segment, and the received packet is read one byte at a time.
 * The Unix time is returned, that is, seconds from 1970-01-01T00:00.
 */


unsigned long inline ntpUnixTime (UDP &udp)
{
  static int udpInited = udp.begin(12333); // open socket on arbitrary port

  const char timeServer[] = "pool.ntp.org";  // NTP server

  // Only the first four bytes of an outgoing NTP packet need to be set
  // appropriately, the rest can be whatever.
  const long ntpFirstFourBytes = 0xEC0600E3; // NTP request header

  // Fail if WiFiUdp.begin() could not init a socket
  if (! udpInited)
    return 0;

  // Clear received data from possible stray received packets
  udp.flush();

  // Send an NTP request
  if (! (udp.beginPacket(timeServer, 123) // 123 is the NTP port
  && udp.write((byte *)&ntpFirstFourBytes, 48) == 48
    && udp.endPacket()))
    return 0;				// sending request failed

  // Wait for response; check every pollIntv ms up to maxPoll times
  const int pollIntv = 150;		// poll every this many ms
  const byte maxPoll = 15;		// poll up to this many times
  int pktLen;				// received packet length
  for (byte i = 0; i < maxPoll; i++) {
    if ((pktLen = udp.parsePacket()) == 48)
      break;
    delay(pollIntv);
  }
  if (pktLen != 48)
    return 0;				// no correct packet received

  // Read and discard the first useless bytes
  // Set useless to 32 for speed; set to 40 for accuracy.
  const byte useless = 40;
  for (byte i = 0; i < useless; ++i)
    udp.read();

  // Read the integer part of sending time
  unsigned long time = udp.read();	// NTP time
  for (byte i = 1; i < 4; i++)
    time = time << 8 | udp.read();

  // Round to the nearest second if we want accuracy
  // The fractionary part is the next byte divided by 256: if it is
  // greater than 500ms we round to the next second; we also account
  // for an assumed network delay of 50ms, and (0.5-0.05)*256=115;
  // additionally, we account for how much we delayed reading the packet
  // since its arrival, which we assume on average to be pollIntv/2.
  time += (udp.read() > 115 - pollIntv / 8);

  // Discard the rest of the packet
  udp.flush();

  return time - 2208988800ul + timeZone * SECS_PER_HOUR;		// convert NTP time to Unix time
}


