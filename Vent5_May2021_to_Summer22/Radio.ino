#include <RelayXBee.h>
#include <SoftwareSerial.h>

//SoftwareSerial port = SoftwareSerial(3,4);//Connect on pins 2-3
#define xbeeSerial Serial4

RelayXBee xbee = RelayXBee(&xbeeSerial, "UMN1");

unsigned long int xbeeTimer = 0;
String comm = " ";

void radioSetup() {
  Serial.println("Setting up the the Xbee Radio");
  char xbeeChannel = 'B';
  xbeeSerial.begin(9600);   // Xbee Baud rate is 9600
  xbee.init(xbeeChannel);   // Need to make sure xbees on both ends have the same identifier. This sets "AAAA"
  xbee.enterATmode();       // Allows the setting of xbee parameters
  xbee.atCommand("ATDL0");
  xbee.atCommand("ATMY1");
  xbee.exitATmode();
  Serial.println("Finished Setting up the Xbee Radio");
  xbeeSerial.write("Hi, I'm the vent radio and I am paired and working");
}

void updateRadio() {
  if (altFeet < 50000 || (jigglingClose == true) || (jigglingOpen == true)) { // limits radio comms to under 50,000 ft 
    if (xbeeSerial.available() > 0 || (jigglingClose == true) || (jigglingOpen == true)) {
      comm = xbeeSerial.readString(); // receives the radio comms and collects it in the string comm
      Serial.println(comm);
      xbeeSerial.flush();
      if (comm == "JIGGLE" || (jigglingClose == true) || (jigglingOpen == true)) {
        Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
        Serial.println("Jiggle Message received");
        if ((flapperState.equals("Closed")) || (jigglingClose == true)) {
          Serial.println("We are about to do close jiggling");
          
          // opens the vent, waits one cycle, then closes the vent
          if (jigglingClose == false) {
            Serial.println("Starting the close jiggling");
            openVent();
            jigglingClose = true;
            commandTime = currTimeS;
          }
          if ((commandTime + 2 < currTimeS) && (jigglingClose == true)) {
            Serial.println("Finishing the close jiggling");
            closeVent();
            jigglingClose = false;
          }
        }
        else {
          if (jigglingOpen == false) {
            Serial.println("Starting the open jiggling");
            closeVent();
            jigglingOpen = true;
            commandTime = currTimeS;
          }
          if ((commandTime + 2 < currTimeS) && (jigglingOpen == true)) {
            Serial.println("Finishing the open jiggling");
            closeVent();
            jigglingOpen = false;
          }
        }
      }
      Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
      if (comm == "RATE") {
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        Serial.println("Ascent Rate Message received");
        Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        xbeeSerial.print("|" + String(ascent_rate) + ", " + String(avg_ascent_rate) + ", " + String(gps_ascent_rate) + ", " + String(press_ascent_rate));
      }
    }
  }
  if ((millis() - xbeeTimer) > 1000 * 60) {
    xbeeTimer = millis();
    Serial.println("///////////////////////////////////////////////////");
    Serial.println(String(xbeeTimer / (1000*60)) + "min: Sending Radio Data");
    Serial.println("///////////////////////////////////////////////////");
    xbeeSerial.print(radioData);
  }

}
