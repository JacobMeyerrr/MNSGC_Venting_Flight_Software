// By Jacob Meyer
// Edited by Benjamin Brynestad, Panos Delton, Shea Larson, Ethan Thompson-Jewell, Seyon Wallo
// state_machine by Nathan Pharis, Steele Mitchell
// GPS Emulation by Benjamin Stevens


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
#include <MS5611.h>

////////////////////////**********Global Variables*********///////////////////////////
////////////////////////**********Global Variables*********///////////////////////////
////////////////////////**********Global Variables*********///////////////////////////
////////////////////////**********Global Variables*********///////////////////////////
////////////////////////**********Global Variables*********///////////////////////////

// Ublox-8 GPS Macros // New Macros
#define ubloxSerial Serial5 //communication channel for UBLOX GPS
#define gpsTolerance 25     //ft/s the gps is allowed to drift from last contact
UbloxGPS gps = UbloxGPS(&ubloxSerial); //creates object for GPS tracking

// SD Card
#define chipSelect BUILTIN_SDCARD // "BUILDIN_SDCARD" indicates that you're using the onboard Teensy 3.5 SD Logger
String header = "Flight Time(s),Time (s),Time (millis),GPS Hr,GPS Min,GPS Sec,Battery Temperature (F),Pressure (psi),Pressure Altitude(feet),Suggested Bounds State,Current Bounds State,Lat,Long,Altitude (feet),# of Satellites,Valid Ascent Rate (m/s),Avg. Ascent Rate (m/s),GPS Ascent Rate,Pressure Ascent Rate,Heater State,Flapper State,Measured Servo Position (degrees),Commanded Servo Position,Open Flapper Servo Position,Closed Flapper Servo Position,Cutter State,Suggested State,Current State,Vent Reason";
File datalog;
File datalogIMU;
char filename[] = "SDCARD00.csv"; // Default name of the SD log
bool sdActive = false; // will set to true later if an SD card is detected

// Servo variables
#define FEEDBACK_PIN A7 // This is the servo feedback pin for the flapper opening/closing servo (might want to double check with schematic), measures servo position
#define PWM_PIN 20 // This is the pin that controls the flapper servo on our PCB
Servo ventServo; // just a variable name for the venting system's flapper servo
unsigned int serialByte;
float servoFeedback; // instantaneous servo position value
int openServo = 0; // The measured servo position for when the vent is fully open
int closeServo = 180; // The measured servo position for when the vent is fully closed    THIS MUST BE CHECKED FOR EACH NEW AVIONICS/MECHANISIM PACKAGE

////////////////////////**********End Of Global Variables*********///////////////////////////
////////////////////////**********End Of Global Variables*********///////////////////////////
////////////////////////**********End Of Global Variables*********///////////////////////////
////////////////////////**********End Of Global Variables*********///////////////////////////
////////////////////////**********End Of Global Variables*********///////////////////////////

//CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT
//CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT
//CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT

#define EASTERN_BOUNDARY -108.9 // Broadview
#define WESTERN_BOUNDARY -110.25 // Springdale
#define SOUTHERN_BOUNDARY 45.70 // Springdale
#define NORTHERN_BOUNDARY 46.10 // Broadview

bool emulationCheck = false; ///MAKE TRUE IF IN EMULATION, THIS WILL ALLOW FOR COMMUNICATION TO THE EMULATOR
float desieredRateReduction = 10; //this is the how much you would like to reduce the ascent rate by in precentange (%)
int preventLengthS1 = 120; // in seconds        only prevent in flight for 2 mins
int preventLengthS2 = 30; // in seconds         NOT CURRENTLY SET FOR Bozeman: only one preventing on this flight
int preVentAlt1 = 60000; // in feet(ft)         only prevent in flight at 60k feet
int preVentAlt2 = 80000; // in feet(ft)         NOT CURRENTLY SET FOR Bozeman: only one preventing on this flight
int bigVentAlt = 90000; //in ft                 NOT CURRENTLY SET FOR Bozeman: no big venting on this flight
int preVentTimeS1 = 80 * 60;//NOT CURRENTLY SET FOR Bozeman: the backup is not timed but based on pressure altitude
int preVentTimeS2 ; //NOT CURRENTLY SET FOR Bozeman: the backup is not timed but based on pressure altitude
int terminateAlt = 85000;
int terminateTimeS = 240 * 60; //this is time to terminate flight "master timer" usually set to 4 hours

float targetAscentRateAfterBigVent = 3; //Not currently used
//CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT
//CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT
//CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT CHANGE BEFORE FLIGHT

///////////////////////**********Timer and misc. variables********///////////////////////////
///////////////////////**********Timer and misc. variables********///////////////////////////
///////////////////////**********Timer and misc. variables********///////////////////////////
///////////////////////**********Timer and misc. variables********///////////////////////////
///////////////////////**********Timer and misc. variables********///////////////////////////


bool ventSent = false;  //bool that if true the vent has communicated with emulator that vent is open if true, false if closed

unsigned long prevTime = 0; // Typical for control flow in most MURI ballooning lab codes, this is called at the end of every 2-second cycle
String message = ""; // the variable that is logged to the SD card every cycle
String radioData = "";
unsigned long interval = 2000; // units of milliseconds, defines how much time is between each cycle of the main loop (currently 2 seconds before each loop + datalog)
int currTimeS = 0; // the current time in seconds, usually defined as millis()/1000;
int timeBeforeFlight = 1000000;
int flightTime = 0; 
double targetAscentRate;
double beforeRatePV1;
double beforeRatePV2;// before the ascent rate is reduced by venting
double beforeBigVentRate;
double ascent_rate = 5; // ascent rate in meters per second (m/s), set it to 5 for the first 10 datapoints (after the flight is detected to "begin" at 5000 feet) so it doesn't prematurely set off the cutter
double gps_ascent_rate = 5;
double press_ascent_rate = 5;
double last_twenty_ascent_rates[20] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}; // last 20 ascent rates in meters per second (m/s), set it to 5 for the first 20 datapoints (after the flight is detected to "begin" at 5000 feet) so it doesn't prematurely set off the cutter
double avg_ascent_rate; // this is the average of the last 10 ascent rates (see above); this is the one that's used in decisions since individual ascent rates can be noisy; avoids extraneous data points
unsigned long prev_time2 = 0; // similar to prev_time, EXCEPT that prev_time2 is the last time that the GPS got a lock, this is so that the ascent rate can be calculated properly
double prev_Press_Altitude;
double prev_Control_Altitude; // this is the altitude reading from the GPS the last time it had a lock, corresponds to the time at prev_time2
String GPSdata; // A string to put all of the relavant GPS data in (lat, long, alt, etc.)
int numSats;
String heaterState; // is the battery heater on or off? (not used in Spring 2021 venting system; heater always on since it's plugged directly into battery; lasts about 6 hours (with a 9v) when fully on
unsigned long timeAtPreVenting1S = 1000000000; // The time (in seconds) when you reach the first pre-venting; initial value set to infinity so that it doesn't prematurely set off anything in the logic
bool AlreadyStartedPreVenting1 = false; // tells you whether you've started pre-venting 1 or not, helps with logic flow
bool AlreadyFinishedPreVenting1 = false; // tells you whether you've finished the first pre-vent or not, helps with logic flow
unsigned long timeAtPreventing2S = 100000000000; // the time (seconds) when you reach the second pre-venting; initial time set to infinity to prevent premature logic failiure
bool AlreadyStartedPreventing2 = false; // same function as pre-venting 1's booleans
bool AlreadyFinishedPreventing2 = false; // same function as pre-venting 1's booleans
unsigned long timeAtBigVent = 10000000000; // The time (in seconds) at which you reach your permanent venting (close to 120,000 feet)
bool AlreadyStartedBigVent = false; // same function as the pre-venting booleans
bool AlreadyFinishedBigVent = false; // same function as the pre-venting booleans
bool AlreadyStartedVentToFloat= false;
bool AlreadyFinishedVentToFloat = false;
String cutterState = "OFF"; // this tells us the state of the cutters (on or off?) and the reason why they terminated/cut or tried to cut; this is logged to the SD card every cycle
String flapperState; // this variable tells you whether the flapper supposed to be open or closed. this is logged to the SD card every cycle.
bool GPS_LOCK = false; // is the GPS locked? this is very important. You won't get the regular LED flashing pattern if the GPS is unplugged or isn't able to be set to airborne mode.
bool FlightHasBegun = false;
bool fivekFeetReached = false;// this is triggered once the balloon/vent crossed 5000 feet altitude to prevent the balloon from prematurely cutting/terminating/venting below 5000 feet; no logic executed before it crosses 5000 feet.
unsigned long timeSince5kFeetS = 0; // this is the time in seconds that the balloon crosses 5000 feet; very useful for backup timers (used in the "5000 feet timer" currently)
String serialcommand = ""; // If you plug your computer into the teensy with the arduino IDE serial monitor open, you can send commands (see the list in the main loop) via the serial cable.
bool Res1Burned = false; // tells you whether you've burned resistor 1 for its first 7-second interval yet
bool Res2Burned = false; // tells you whether you've burned resistor 2 for its first 7-second interval yet
bool Res1Burned2 = false;// tells you whether you've burned resistor 1 for its first 30-second interval yet
bool Res2Burned2 = false;// tells you whether you've burned resistor 2 for its first 30-second interval yet (the overall burn sequence is: 7s R1 - 7s R2 - 30s R1 - 30s R2 - then resistors stop burning)
bool Res1Burned3 = false;
int timesResistor2Burned = 0;
int burnTime1StartS; // when resistor 1 started burning for the current interval so that you can set the 7 and 30 second timers
int burnTime2StartS; // when resistor 2 started burning for the current interval so that you can set the 7 and 30 second timers
bool Res1on = false; // tells you whether resistor 1 is currently burning or not
bool Res2on = false; // tells you whether resistor 2 is currently burning or not
float latitude, lon, altFeet; //GPS variables for positioning
int gpsHR, gpsMIN, gpsSEC; // GPS variables for time stamp
int preVentingTimeS1; //Acts as the count for the first pre-vent
int preVentingTimeS2; //Acts as the count for the second pre-vent
int bigVentTimeS;
int ventToFloatTime;
int commandTime;
int finishventTimeS1 = 10000000; // saves the time when the first pre-vent finished
int finishventTimeS2 = 10000000; // saves the time when the second pre-vent finished
int finishBigVentTimeS = 1000000;
int finishVentToFloat = 1000000;

String ventReason = "None"; //Used to write to SD card the reason for the venting
float avgAscentRateAt30PV1 = -999; //this and the following variables are recorded for calculations in future venting during flight
float avgAscentRateAt60PV1 = -999;
float avgAscentRateAt90PV1 = -999;
float avgAscentRateAt120PV1;

float avgAscentRateAt30PV2 = -999;
float avgAscentRateAt60PV2 = -999;
float avgAscentRateAt90PV2 = -999;
float avgAscentRateAt120PV2;


int terminationStartTimeS; //20 second timer to display LED patterns before terminating
bool terminationBegun = false; //Once termination is called, this is set to true to start the termination timer
int descentTimer;//Timer for the rare case that the balloon is in descent before the final vent
bool descentTimerStarted = false; //boolean used to set the descentTimer
int backupFloatTimer = 1000000;
bool backupFloatTimerStarted = false;
int floatTimer = 1000000; //Timer for the rare case that the balloon is in float before the final vent
bool floatTimerStarted = false; //boolean used to set the floatTimer
int jiggleTimer; //Timer to help pace the servo / tell whether the servo needs to be "jiggled" if it's out of place
bool jiggleTimerStarted = false; //boolean used to set the jiggleTimer

MS5611 baro;
float msPressure;
float prevPress;
int pressCount;
double pressureAltFeet; // pressure altitude in feet
float pressureBoundary1;
float pressureBoundary2;
float pressureBoundary3;

//////////////////////////////////////Radio Variables////////////////////////////////////////////
bool jigglingClose = false;
bool jigglingOpen = false;

///////////////////////**********Added state_machine variables********///////////////////////////
///////////////////////**********Added state_machine variables********///////////////////////////
///////////////////////**********Added state_machine variables********///////////////////////////
///////////////////////**********Added state_machine variables********///////////////////////////
///////////////////////**********Added state_machine variables********///////////////////////////
#define INITIALIZATION 0x00
#define ASCENT 0x01
#define SLOW_ASCENT 0x02
#define FLOAT 0x03
#define SLOW_DESCENT 0x04
#define DESCENT 0x05
#define FAST_DESCENT 0x06
#define PAST_TIMER 0x09
#define ERROR_STATE 0x10

#define OUT_OF_BOUNDS 0x08
#define IN_BOUNDS 0x11;

String suggestedState = "Initialization";
String stateReturn;
uint8_t stateSuggest; // state recommended by control
uint8_t currentState = INITIALIZATION; // state we are in, starts as initialization
uint8_t SDcounter = 0;
uint8_t ascentCounter = 0, SAcounter = 0, floatCounter = 0, descentCounter = 0, fastDescentCounter = 0;
uint8_t tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0;
uint8_t boundsSuggest = IN_BOUNDS;
uint8_t boundsCurrent = IN_BOUNDS;
String boundsReturn = "";
int boundsCounter = 0;

#define M2MS 2*60000
#define ALTITUDE_FLOOR 5000
#define ALTITUDE_CEILING 100000
#define SA_FLOOR 50000
#define SLOW_DESCENT_FLOOR 80000


#define MASTER_TIMER 180*M2MS
#define ASCENT_TIMER 150*M2MS
#define SA_TIMER 30*M2MS
#define FLOAT_TIMER 30*M2MS
#define SLOW_DESCENT_TIMER 40*M2MS
#define INITIALIZATION_TIME 25*M2MS
#define DEFAULT_TIME 30*M2MS

// float state ascent rate fixes
float ARprev = 0;
int FARcounter = 0;
unsigned long updateStamp, SDstamp, descentStamp, timerStampCutA;
int which = 1;
int initCounter = 1;
int initCounter2 = 1;
int timeCounter = 1;
unsigned long ascentStamp = 0, SAstamp = 0, floatStamp = 0, defaultStamp = 0, defaultStamp2, defaultStampCutA = 0, xbeeStamp = 0;
uint8_t cutReasonA = 0x22, cutReasonB = 0x22;

///////////////////////**********End Of Timer and misc. variables********///////////////////////////
///////////////////////**********End Of Timer and misc. variables********///////////////////////////
///////////////////////**********End Of Timer and misc. variables********///////////////////////////
///////////////////////**********End Of Timer and misc. variables********///////////////////////////
///////////////////////**********End Of Timer and misc. variables********///////////////////////////

// code to initialize our system (runs only once)
void setup() {
  systemSetup();
}

// main loop and state machine (runs repeatedly)
void loop() {
  systemUpdate();
}
