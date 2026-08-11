// Venting System 1.5 Flight Computer (For 1/06/2021 Flight Originally, repurposed for 1/17/2021 and 2/6/2021 flights)
// By: Jacob Meyer (Aerospace Eng., Class of 2021)

// Libraries
  // SD Card
    #include <SD.h>
  // Servo
    #include <Servo.h>
    #include <Arduino.h>
    #include <SPI.h>
  // UBLOX-8 GPS
    #include <SoftwareSerial.h>
    //#include <TinyGPS++.h> // Old Libraries
    #include <UbloxGPS.h>
// Sensors and Pins (add later)

////////////////////////**********Global Variables*********///////////////////////////
 

// UBLOX-8 GPS Macros // Old Macros
//  //SoftwareSerial serial_connection(9, 10); //RX=pin 6, TX=pin 7 on Nano
//  #define serial_connection Serial5
//  TinyGPSPlus gps;//This is the GPS object that will pretty much do all the grunt work with the NMEA data
//  int numSatellites = 0;

// Ublox-8 GPS Macros // New Macros
  #define ubloxSerial Serial5 //communication channel for UBLOX GPS
  #define gpsTolerance 25     //ft/s the gps is allowed to drift from last contact
  UbloxGPS gps = UbloxGPS(&ubloxSerial);                          //creates object for GPS tracking
 
// SD Card
  #define chipSelect BUILTIN_SDCARD //Should highlight if you have teensy 3.5/3.6/4.0 selected
  String header = "Time (s),Battery Temperature (C),PCB Temperature (C),Pressure (psi),Pressure Altitude (feet),Lat,Long,Altitude (feet),# of Satellites, Rate (m/s),Heater State,Vent State,Measured Servo Position (degrees),Cutter State,Estimated Altitude (From last known GPS data)";
  File datalog;
  File datalogIMU;
  char filename[] = "SDCARD00.csv";
  bool sdActive = false;

// Servo
  #define FEEDBACK_PIN A2 //A1 on nano
  #define PWM_PIN 6 // 5 on nano
  Servo ventServo;
  unsigned int serialByte;
  float servoFeedback;

// Release Servo
//  //#define FEEDBACK_PIN A7 //A1 on nano
//  #define PWM_PIN2 5 // 5 on nano
//  Servo releaseServo;
//  unsigned int serialByte2;
//  //float servoFeedback;

// Timer and misc.
  unsigned long prevTime = 0;
  String message = "";
  unsigned long interval = 2000;
  String batteryTempl = "";
  String pcbTempl = "";
  String pressurel = "";
  int currTimeS = 0;
  double Control_Altitude;
  double ascent_rate = 5;
  double last_ten_ascent_rates[10] = {5,5,5,5,5,5,5,5,5,5};
  double avg_ascent_rate;
  unsigned long prev_time2=0;
  double prev_Control_Altitude;
  String GPSdata;
  //int numSats;
  String heaterState;
  unsigned long timeAt80kS=100000000000;
  bool AlreadyReached80K = false;
  bool AlreadyFinished80k = false;
  unsigned long timeAt90kS=100000000000000;
  bool AlreadyReached90K = false;
  bool AlreadyFinished90k = false;
  unsigned long timeAt100kS=10000000000000;
  bool AlreadyReached100K = false;
  bool AlreadyFinished100k = false;
  String cutterState = "OFF";
  String flapperState;
  bool NinetyKFeetpAltReached = false;
  double pressureAltFeet;
  bool GPS_LOCK = false;
  bool FlightHasBegun=false;
  unsigned long timeSince5kFeetS = 0;
 // int altFeet2 = 900; // !!!!!!!!!!!!!!!!!!!!!!!!!!!! FOR TESTING PURPOSES ONLY. DELETE WHEN DONE TESTING!!!! REMOVE THIS BEFORE FLIGHT!!!!!!!!!!!!    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  String serialcommand = ""; // FOR TESTING ONLY?
  double ARat80Kfeet;
  double ARafter80kfeet;
  int NewOpeningTimeS;
  bool Res1Burned = false;
  bool Res2Burned = false;
  bool Res1Burned2 = false;
  bool Res2Burned2 = false;
  int burnTime1StartS;
  int burnTime2StartS;
  bool NeedToRecalculate=false;
  int est_altitude;
  int lastAltFeet;
  bool Res1on = false;
  bool Res2on = false;
  
// Variables Unique to Vent #2
bool opened_vent = false;
int openingTime_s = 100*60; // set to infinity initially so the math works
bool GPS_LOCKED = false;
int System_Time_Min = 0;

// Heater/Latching-Relay Funcitons (no external libraries)2

//// UBLOX-8 GPS (Old ++ Library)
//
//void initGPS()
//{
//  Serial.print("starting ublox setup... ");
//  serial_connection.begin(9600);//This opens up communications to the GPS
//  while(!setAirborne()){
//    Serial.println("Airborne mode not set...");
//  }
//  Serial.println("GPS Start");//Just show to the monitor that the sketch has started
//  Serial.println("ublox setup complete");
//}

//sends calibration message to force module into airborne mode (DO NOT USE!) - FAILED ATTEMPT TO GET IT WORKING DIRECTLY WITH TINYGPS++
//bool setAirborne() {
//  byte airMode[] = {0xB5,0x62,0x06,0x24,0x24,0x00,0xFF,0xFF,0x06,0x02,0x00,0x00,0x00,0x00,0x10,0x27,0x00,
//            0x00,0x05,0x00,0xFA,0x00,0xFA,0x00,0x64,0x00,0x2C,0x01,0x00,0x00,0x00,0x00,0x10,
//            0x27,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4C,0xBA};
//  byte length = 44;
//  byte response[10];
//  byte acknowledge[] = {0xB5, 0x62, 0x05, 0x01, 0x02, 0x00, 0x06, 0x24, 0x32, 0x5B};
//  bool ack = true;
//  //serial_connection.update();
//  serial_connection.write(airMode, length);
//  //gps.encode(airMode);
//  delay(100);
//  for (byte i=0; i < 10; i++) {
//    response[i] = serial_connection.read();
//  }
//  for (byte i=0; i < 10; i++) {
//    if (response[i] != acknowledge[i]) ack = false;
//  }
//  Serial.println("ack");
//  Serial.println(ack);
//  return ack;
//}

//// Built-in Teensy SD-Card Data Logger

void sdSetup(){
  pinMode(chipSelect,OUTPUT);
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present");
//    updateOled("Turn off\nand Insert SD card");
    for(int i=1; i<20; i++){
      digitalWrite(13,HIGH);
    //  digitalWrite(sdLED,LOW);
      delay(100);
      digitalWrite(13,LOW);
    //  digitalWrite(sdLED,HIGH);
      delay(100);
    }
  }
  else {
    Serial.println("Card initialized.\nCreating File...");
    for (byte i = 0; i < 100; i++) {
      filename[6] = '0' + i/10;
      filename[7] = '0' + i%10;
      if (!SD.exists(filename)) {
        datalog = SD.open(filename, FILE_WRITE);
        sdActive = true;
        Serial.println("Logging to: " + String(filename));
      //  updateOled("Logging:\n\n" + String(filename));
        delay(1000);
        break;}
    }
    if (!sdActive) {
      Serial.println("No available file names; clear SD card to enable logging");
//      updateOled("Clear SD!");
      delay(5000);
    }
    logData(header);
  }
}

void logData(String Data){
  datalog = SD.open(filename, FILE_WRITE);
  datalog.println(Data);
  datalog.close();
  Serial.println(Data);
}

void openVent() {
  ventServo.write(0);
  //releaseServo.write(0);
  flapperState = "Open";
  // determine which LED to turn on depending on if a command was received or not
 /* if(commandOverride) {
   // digitalWrite(COMMAND_LED,HIGH);
   // digitalWrite(AUTO_LED,LOW);
    Serial.println("Command opening...");
  }
  else {
   // digitalWrite(AUTO_LED,HIGH);
   // Serial.println("Auto opening...");
  }

  //ventOpen = true;

  //openStamp = millis();
*/
}


void closeVent() {
  ventServo.write(128);
  //releaseServo.write(0);
  flapperState = "Closed";
/*
  //commandOverride = false;
  //ventOpen = false;

  Serial.println("Closing vent...");
  // turn off both LEDs
 // digitalWrite(COMMAND_LED,LOW);
 // digitalWrite(AUTO_LED,LOW);
  */
}

//void releaseVent() {
//  releaseServo.write(0);
//  cutterState = "Released";
//  // determine which LED to turn on depending on if a command was received or not
// /* if(commandOverride) {
//   // digitalWrite(COMMAND_LED,HIGH);
//   // digitalWrite(AUTO_LED,LOW);
//    Serial.println("Command opening...");
//  }
//  else {
//   // digitalWrite(AUTO_LED,HIGH);
//   // Serial.println("Auto opening...");
//  }
//
//  //ventOpen = true;
//
//  //openStamp = millis();
//*/
//}
//
//void unReleaseVent() {
//  releaseServo.write(180);
//  cutterState = "";
//  // determine which LED to turn on depending on if a command was received or not
// /* if(commandOverride) {
//   // digitalWrite(COMMAND_LED,HIGH);
//   // digitalWrite(AUTO_LED,LOW);
//    Serial.println("Command opening...");
//  }
//  else {
//   // digitalWrite(AUTO_LED,HIGH);
//   // Serial.println("Auto opening...");
//  }
//
//  //ventOpen = true;
//
//  //openStamp = millis();
//*/
//}

int servoPos() // obtain the position of the servo through the analog pin
{
  int val = analogRead(FEEDBACK_PIN);            // Feedback is pin A2 on the current Teensy 3.5 / PCB setup - reads the value of the potentiometer (value between 0 and 1023)
  val = map(val, 0, 1023, 0, 179);     // scale it to use it with the servo (value between 0 and 180)
  return val;
}

// Two 10-Ohm Resistors and L9110H H-drivers to Pop The Balloon
void popBalloon()
{
  currTimeS = millis()/1000; // Timing may be screwed up without this for whatever reason
  openVent(); // Every time you try to pop, keep the vent open as a backup
  if(!Res1Burned)
  {
    if(!Res1on)
    {
      burnTime1StartS = currTimeS;
    }
    burnResistor();
    if(currTimeS-burnTime1StartS>7.9)
    {
      stopBurn();
      Res1Burned = true;
    }
  }
  else if(!Res2Burned)
  {
    if(!Res2on)
    {
      burnTime2StartS = currTimeS;
    }
    burnResistor2();
    if(currTimeS-burnTime2StartS>7.9)
    {
      stopBurn2();
      Res2Burned = true;
    }
  }
  else if(!Res1Burned2)
  {
    if(!Res1on)
    {
      burnTime1StartS = currTimeS;
    }
    burnResistor();
    if(currTimeS-burnTime1StartS>35)
    {
      stopBurn();
      Res1Burned2 = true;
    }
  }
  else if(!Res2Burned2)
  {
    if(!Res2on)
    {
      burnTime2StartS = currTimeS;
    }
    burnResistor2();
    if(currTimeS-burnTime2StartS>35)
    {
      stopBurn2();
      Res2Burned2 = true;
    }
  }
  
}

// LED helper functions
void ledSETUP()
{
  pinMode(23,OUTPUT);
}

void ledON()
{
  digitalWrite(23,HIGH);
}

void ledOFF()
{
  digitalWrite(23,LOW);
}

// Flash the LED in the appropriate pattern, to indicate the state of the vent to any upward facing cameras below
void led()
{
  if(cutterState=="OFF" || cutterState=="")
  {
    if(flapperState=="Closed")
    {
      if(millis()%1000<250)
      {
        ledON();
      }
      else
      {
        ledOFF();
      }
    }
    else if(flapperState=="Open")
    {
      if(millis()%2000<1000)
      {
        ledON();
      }
      else
      {
        ledOFF();
      }
    }
  }
  else
  {
    if(flapperState=="Closed")
    {
//      if(millis()%1000<250)
//      {
//        ledOFF();
//      }
//      else
//      {
        ledON(); // Just keep it on all the time to distinguish easier
//      }
    }
    else if(flapperState=="Open")
    {
      if(millis()%4000<1000)
      {
        ledON();
      }
      else
      {
        ledOFF();
      }
    }
  }
  //KEH
}


// System data collection and state machine

void systemUpdate()
{
// Flash the LED appropriately
  led();
  
// BELOW: FOR TESTING ONLY ******DELETE BEFORE FLIGHT*********REMOVE BEFORE FLIGHT*********

  if(Serial.available()){
        serialcommand = Serial.readStringUntil('\n');
        Serial.print("You typed: " );
        Serial.println(serialcommand);
    }
// ABOVE FOR TESTING ONLY**********DELETE BEFORE FLIGHT*************REMOVE BEFORE FLIGHT**********

// Below: Check for GPS data, update buffer if data available
  GPS_LOCK = true;
  float latitude, lon, altFeet;
  int numSats;

  // New GPS Update
    gps.update();

  //delay(3); // For old GPS library only
// ^^^^^^^^^^ABOVE: Check for GPS data, update buffer if data available^^^^^^^^^^^^^
// FOR TESTING ONLY***REMOVE "*100" BEFORE FLIGHT!!!!!!!!!!!!!!!!!!!!!!!!!************
  /*100x EMULATOR*/ //unsigned long currTime = millis()*100; // Find the current time (very important for logic) // ALERT ALERT EMULATOR************************************************************************100* is TO BE REMOVED BEFORE FLIGHT!!!!*************************************************
// ^^^^^FOR TESTING ONLY***REMOVE "*10" BEFORE FLIGHT!!!!!!!!!!!!!!!!!!!!!!!!!************^^^^^
  unsigned long currTime = millis(); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<REAL THING. Uncomment before flight!>>>>
  if(currTime-prevTime>interval) // Interval is currently 2000 ms (or 2 seconds) as defined
  {
    // **************Get Sensor Data and record it**********************
   // prevTime=millis()*100; // // 100x EMULATOR; FOR TESTING ONLY***REMOVE "*100" BEFORE FLIGHT!!!!!!!!!!!!!!!!!!!!!!!!!*********************************************
    prevTime=millis(); // RE-ACTIVATE BEFORE FLIGHT!!!<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    currTimeS = currTime/1000;
   
      // Data line to log to the CSV file on our SD card
    message = String(currTime/1000) + "," + "," + "," + ",";

// *************************************BELOW FOR TESTING ONLY BELOW****************************************
// *************************************BELOW FOR TESTING ONLY BELOW****************************************
// *************************************BELOW FOR TESTING ONLY BELOW****************************************
// *************************************BELOW FOR TESTING ONLY BELOW****************************************
//if(cutterState!="OFF")
//{
//  if(altFeet2<900)
//  {
//    altFeet2=altFeet2;
//  }
//  else{
//    altFeet2 = altFeet2-41.2;
//  }
//}
////else if(altFeet2 > 12000)
////{
////  altFeet2 = altFeet2-10.0;
////}
////else if(altFeet2 > 0)
////{
////  altFeet2 = altFeet2 + 32.2;
////}
//else if(altFeet2 > 100000)
//{
//  altFeet2 = altFeet2 + 12;
//}
//else if(altFeet2 > 90000)
//{
//  altFeet2 = altFeet2 + 18.7;
//}
//else if(altFeet2 > 80000)
//{
//  altFeet2 = altFeet2 + 24.3;
//}
//else
//{
//  altFeet2 = altFeet2 + 32.2;
//}
//altFeet = altFeet2;
// *************************************ABOVE FOR TESTING ONLY ABOVE****************************************
// *************************************ABOVE FOR TESTING ONLY ABOVE****************************************
// *************************************ABOVE FOR TESTING ONLY ABOVE****************************************
// *************************************ABOVE FOR TESTING ONLY ABOVE****************************************

  if(GPS_LOCK==true)
  {
    float altFeetCheck = gps.getAlt_feet();
    if(altFeetCheck < 850)
    {
      GPS_LOCK=false;
    }
    if(altFeetCheck < 41000)
    {
      if(altFeetCheck > 38500)
      {
        GPS_LOCK = false;
      }
    }
  }

  // *******If GPS Lock is obtained, log that data (if statement), otherwise just fill the csv out with blank spaces (else statement)************
  if(GPS_LOCK==true)
  {
      latitude = gps.getLat();
      lon = gps.getLon();
      altFeet = gps.getAlt_feet();
      numSats = gps.getSats();
//      Serial.println("Trying to print...");
//      Serial.println(latitude);
//      Serial.println(lon);
//      Serial.println(altFeet);
//      Serial.println("Trying to print...");
//      Serial.println(gps.location.lat());
//      Serial.println(gps.location.lng());
//      Serial.println(gps.speed.value());
//      Serial.println(gps.altitude.feet());
      //Get the latest info from the gps object which it derived from the data sent by the GPS unit
     // altFeet = gps.altitude.feet();
/*EMULATOR*/ //altFeet = altFeet2; // <<<<<<<<<<<<<<<<<<<<<<<<<*************************FOR TESTING ONLY FOR TESTING ONLY***********************<<<<<<<<<<<<<<<<<DELETE BEFORE FLIGHT<<<<<<<<<<<<*******************************************
      /*EMULATOR 100x*/ //ascent_rate = 0.3048*(((altFeet - prev_Control_Altitude)/(millis() - prev_time2))*1000); // m/s //***********<<<<***WARNING: REMOVE "/100" BEFORE FLIGHT!!!!!!!!******************************************
      ascent_rate = 0.3048*(((altFeet - prev_Control_Altitude)/(millis() - prev_time2))*1000); // m/s<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<UNCOMMENT BEFORE FLIGHT!
      if(ascent_rate > 10)
      {
        ascent_rate = avg_ascent_rate;
      }
      avg_ascent_rate = 0;
      double j = 0;
      for(int i = 0; i < 10; i++)
      {
        if(i < 9)
        {
          last_ten_ascent_rates[i] = last_ten_ascent_rates[i+1];
          if(last_ten_ascent_rates[i]!=0)
          {
            avg_ascent_rate += last_ten_ascent_rates[i];
            j += 1.00;
          }
        }
        if(i==9)
        {
          last_ten_ascent_rates[i] = ascent_rate;
          if(last_ten_ascent_rates[i]!=0)
          {
            avg_ascent_rate += last_ten_ascent_rates[i];
            j += 1.00;
          }
        }
      }
      avg_ascent_rate = avg_ascent_rate/j;
      Serial.println("Average Ascent Rate (below) is:");
      Serial.println(avg_ascent_rate);
      Serial.println("Average Ascent Rate Printed Above.");
      Serial.println("Last Ten Ascent Rates (below) are:");
      Serial.println(String(last_ten_ascent_rates[0])+","+String(last_ten_ascent_rates[1])+","+String(last_ten_ascent_rates[2])+","+String(last_ten_ascent_rates[3])+","+String(last_ten_ascent_rates[4])+","+String(last_ten_ascent_rates[5])+","+String(last_ten_ascent_rates[6])+","+String(last_ten_ascent_rates[7])+","+String(last_ten_ascent_rates[8])+","+String(last_ten_ascent_rates[9])+",");
      Serial.println("Last Ten Ascent Rates Printed Above.");
      prev_time2 = millis();
      prev_Control_Altitude = altFeet;
      GPSdata = "," + String(latitude, 4) + "," + String(lon, 4) + ","  + String(altFeet) + "," + String(numSats) + "," + String(ascent_rate);
      message += GPSdata;
      if(altFeet > 900)
      {
        lastAltFeet=altFeet;
      }
  }
  else
  {
    message += ",,,"+String(altFeet)+",,"+   String(ascent_rate);
    //message += ",,,,,";
  }
  // **********************DECLARE WHETHER THE GPS HAS HAD A RECENT LOCK OR NOT*****************************************
   if((millis() - prev_time2)/1000 > 300) // if more than 300 seconds (5 minutes) have passed since fresh GPS hits, declare that the GPS has lost its lock, for backup logic purposes
   {
    GPS_LOCKED == false;
    Serial.println("GPS LOCKED IS FALSE FALSE FALSE");
   }
   else
   {
    GPS_LOCKED == true;
    Serial.println("GPS LOCKED IS TRUE TRUE TRUE");
   }
 // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************

    // Start Flight Timer once 5000 feet threshold is surpassed (for the backup flight cutter)
    if(FlightHasBegun==false)
    {
      if(altFeet > 5000) // just to make sure it doesn't 
      {
        FlightHasBegun = true;
        timeSince5kFeetS = currTimeS;
        ascent_rate = 5;
        last_ten_ascent_rates[1] = 5;last_ten_ascent_rates[2] = 5;last_ten_ascent_rates[3] = 5;last_ten_ascent_rates[4] = 5;last_ten_ascent_rates[5] = 5;
        last_ten_ascent_rates[6] = 5;last_ten_ascent_rates[7] = 5;last_ten_ascent_rates[8] = 5;last_ten_ascent_rates[9] = 5;last_ten_ascent_rates[0] = 5;
        avg_ascent_rate = 5;
      }
    }
    Serial.println("FlightHasBegun and timeSince5kFeetS");
    Serial.println(FlightHasBegun);
    Serial.println(String(timeSince5kFeetS));

    // *****Ultimate Emergency Timer Cut in case nothing burned or the vent didn't open for whatever reason before*******
    if(currTimeS > 240*60) // Master Backup timer (cuts 3 hours, 60 minutes (4 hours) after turning on in case all else fails
    {
      popBalloon();
      Serial.println("Released because of backup master timer");
      cutterState = "Released because of backup master timer";
    }

    // *********Backup Flight Cutter*************
    if(FlightHasBegun==true)
    {
     if(currTimeS-timeSince5kFeetS > 220*60) // Sort of redundant with the master timer
     {
       popBalloon();
       Serial.println("Released because of backup flight timer");
       cutterState = "Released because of backup flight timer";
     }
//     if(opened_vent==true&&(currTimeS-openingTime_s)>30*60)
//     {
//      popBalloon();
//      Serial.println("Releasing Normally");
//      cutterState = "Released normally";
//     }
//     if(altFeet > 130000)
//     {
//      popBalloon();
//      Serial.println("Released b/c of 105k");
//      cutterState = "Released b/c of 105k";
//     }
     if(AlreadyReached100K==true && altFeet < 80000)
     {
      popBalloon();
      cutterState = "Popped because you went below 80000 feet upon descent, after opening";
     }
     if(avg_ascent_rate < -8.5) // **WARNING** April 11th, 2021 Flight Addition - Just for testing, as a backup for ensuring that we test termination; **WARNING** might or might not want for future flights? Ask Dr. Flaten. If you vent at high altitudes, it shouldn't be a problem, but if your venting descent rate is pretty fast, you might want to re-consider!
     {
      popBalloon();
      cutterState = "Released because balloon popped but we still want to test the neck-release mechanism";
     }
    }


 //EMULATOR ONLY**********REMOVE BEFORE FLIGHT!!!!!!!!!!!!!!!!!******************//////////////////////////
//if(altFeet2 > 50000)
//{
//  GPS_LOCKED=false;//**********************************************************************************
//}
//^^^^^^^^^^^^^^^EMULATOR ONLY!!!!!!!! REMOVE/DELETE BEFORE FLIGHT!!!!^^^^^^^^^^^



    //int est_altitude = lastAltFeet + avg_ascent_rate*(millis() - prev_time2)/1000; // (For the else statement, if the GPS hasn't had a good lock)
    
    // Regular State Machine
    if(GPS_LOCK==true) // if the GPS has had a hit in the last minute or so, assume that its working
    {
      Serial.println("Going through regular state machine logic (GPS working)");
      if(altFeet > 100000) // This is a temporary venting to help ensure that the balloon stack will reach the desired altitude later in the flight
      {
        oneTimeOpen100Kfeet(currTimeS,avg_ascent_rate);
      }
      if(altFeet > 108268) // Same as 33.0 Km; this is the permanent vent opening altitude
      {
        oneTimeOpen33KM(currTimeS); 
        Serial.println("33 km one-time permanent opening");
      }
    }
    else // If GPS isn't working after one minute or so, resort to a timer-based opening
    { 
      Serial.println("Going through backup state machine logic");
      if(FlightHasBegun==true)
      {
        System_Time_Min = (currTimeS-timeSince5kFeetS)/60; // Time since the balloon crossed 5k feet // Note: Old Logic; Note: this logic relies on the GPS working around ~2500 feet in order to work properly, but not throughout the entire flight
  
        Serial.println("System_Time_Min is:");
        Serial.println(String(System_Time_Min)); 
        if(System_Time_Min > 100) // Vent will probably slow down, but it depends on 
        {
          oneTimeOpen33KM(currTimeS); // Permanent open
          Serial.println("105k feet timer-based opening");
        }

      } 
    }   
   // ****INSERT ALTITUE GPS-BASED AND TIMER-BASED VENTING DECISIONS HERE****

// FOR TESTING ONLY**********DELETE BEFORE FLIGHT*************REMOVE BEFORE FLIGHT**********
  if(serialcommand=="OPEN")
  {
    Serial.println("Testing");
    openVent();
    serialcommand=="";
  }
  if(serialcommand=="CLOSE")
  {
    closeVent();
    serialcommand=="";
  }
  if(serialcommand=="BURN")
  {
    burnResistor();
    cutterState = "Burned";
    serialcommand=="";
  }
  if(serialcommand=="SBURN")
  {
    cutterState = "Stopped Burning";
    stopBurn();
    serialcommand=="";
  }
  if(serialcommand=="BURN2")
  {
    burnResistor2();
    cutterState = "Burned2";
    serialcommand=="";
  }
  if(serialcommand=="SBURN2")
  {
    cutterState = "Stopped Burning 2";
    stopBurn2();
    serialcommand=="";
  }
  if(serialcommand=="LEDON")
  {
    digitalWrite(23,HIGH);
    serialcommand=="";
  }
  if(serialcommand=="LEDOFF")
  {
    digitalWrite(23,LOW);
    serialcommand=="";
  }

    message += ",,"+flapperState+","+String(servoPos())+","+String(cutterState)+","+String(est_altitude);
      Serial.println(message);
      Serial.println("");
      // SD Card
      if (true) {
        logData(message);                                        //close file afterward to ensure data is saved properly
      }
 }
}

// Resistor Cutter setup and use functions

//Setup Resistor Cutter 1 and L9110H H-driver
void burnSetup()
{
  // Set the burn pins to output so they actually work
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  // Make sure that they're set to OFF
  digitalWrite(4, LOW); 
  digitalWrite(5, LOW); 
  //End Resistor Cutter Setup
}

// Function for initiating the burn of the resistor
void burnResistor()
{
  //digitalWrite(2, HIGH); //DON'T BURN BOTH HIGH AT ONCE! ONLY ONE OF THE TWO HIGH FOR BURNING!
  digitalWrite(4, HIGH); 
  Serial.println("BURNING RESISTOR NUMBER ONE!!!");
  cutterState = "Burning Res1";
  Res1on = true;
}

// function for manually stopping the burn of the resistor cutter
void stopBurn()
{
  digitalWrite(4, LOW); 
  digitalWrite(5, LOW); 
  cutterState = "Res1 Stopped Burning";
  Res1on = false;
}

//Setup Resistor Cutter 2 and L9110H H-driver
void burn2Setup()
{
  // Set the burn pins to output so they actually work
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  // Make sure that they're set to OFF
  digitalWrite(6, LOW); 
  digitalWrite(7, LOW); 
  //End Resistor Cutter Setup
}

// Function for initiating the burn of the resistor
void burnResistor2()
{
  Res2on = true;
  //digitalWrite(2, HIGH); //DON'T BURN BOTH HIGH AT ONCE! ONLY ONE OF THE TWO HIGH FOR BURNING!
  digitalWrite(6, HIGH); 
  Serial.println("RESISTOR 2 BURNING!!!!!");
  cutterState = "Burning Res2";
  Res2on = true;
}

// function for manually stopping the burn of the resistor cutter
void stopBurn2()
{
  digitalWrite(6, LOW); 
  digitalWrite(7, LOW); 
  cutterState = "Res2 Stopped Burning";
  Res2on = false;
}

// additional helper functions

// Function to (hopefully) increase our balloon's altitude ceiling, so that we don't prematurely burst!
void oneTimeOpen100Kfeet(unsigned long currTimeS, double avg_ascent_rate)
{
  if(!AlreadyReached80K)//Sorry for the notation mix-up. I didn't want to mess up the logic though, lol. 80K booleans refers to 100k altitude and 100k booleans refer to the permanent venting @ 33 km (108K feet in this code - sorry for the mix-up!)
  {
    openVent();
    //delay(30000);
    //closeVent();
    timeAt80kS = currTimeS;
  }
  else 
  {
    if((avg_ascent_rate < 3.9) && !AlreadyFinished80k) // !!!!!!!!!!!!!!!!!!!!!!!!!!!
    {
      closeVent();
      AlreadyFinished80k = true;
    }
  }
  AlreadyReached80K = true;
}

// Function for main Vent opening (permanent opening)

void oneTimeOpen33KM(unsigned long currTimeS)
{
  if(!AlreadyReached100K)
  {
    openVent();
    //delay(30000);
    //closeVent();
    timeAt100kS = currTimeS;
  }
  else
  {
    if(((currTimeS-timeAt100kS)>3600) && !AlreadyFinished100k) // !!!!!!!!!!!!!!!!!!!!!!!!!!!
    {
      //closeVent();
      popBalloon();
      AlreadyFinished100k = true;
    }
  }
  AlreadyReached100K = true;
}

void systemSetup()
{
  Serial.begin(9600); // temporary, probably don't use on flight b/c of the gps
  delay(5000);
  pinMode(23,OUTPUT);
  digitalWrite(23,HIGH);
  delay(5000);
  digitalWrite(23,LOW);
  //Resistor-Cutter / L9110H H-driver Setup
    burnSetup();
  //Resistor-Cutter / L9110H H-driver Setup
    burn2Setup();
    
//  // UBLOX-8 GPS (w/Tiny GPS Library)
//    initGPS();
  
  // UBLOX-8 GPS (UbloxGPS Library)
    Serial.println("Beginning GPS Setup");
    Serial5.begin(UBLOX_BAUD);//starts communication to UBLOX
    gps.init();
    while(!gps.setAirborne()) // loop should terminate once it's set to airborne mode, check with Nathan though
    {
      Serial.println("Not Set to AIrborne Mode");
    }
    Serial.println("Set to airborne mode");
    Serial.println("GPS Setup Ended");

  // Built-in Teensy SD Card
    sdSetup();

  // Servo/Flapper
    ventServo.attach(20);    // initialze servo
    //                                                                                                                                                                                                                                       \
    \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
    openVent();
    //delay(1500);
    closeVent();
    delay(500);

   // release Servo
//    releaseServo.attach(5);    // initialze servo
//    unReleaseVent();
//    delay(500);
//   // releaseVent();
//   // delay(2000);
//    unReleaseVent();
//    delay(500);
    
 
  Serial.println("Beginning!");
  
}

// defunct function
void stateMachine() // separate state machine good practice? Or just keep in system function?
{

//  //determine state and log why you're in that state
//  if(altFeet>80000 && altFeet-80000<500)
//  {
//    openVent(); // open for 30 seconds
//  }
//  if(altFeet>90000 && altFeet-90000<500)
//  {
//    openVent 30 seconds
//  }
//  if(altFeet>100000)
//  {
//    openVent indefinitely
//  }
//  if(((millis()/1000)/60) > BACKUP_CUT_MINUTES)
//  {
//    cut();
//  }
//  if(batteryTemp < 40) // if heating for batteries fails, perform an emergency cut
//  {
//    cut();
//  }
//  
}

void setup() {
  systemSetup();
}


void loop() {
  systemUpdate();
}
