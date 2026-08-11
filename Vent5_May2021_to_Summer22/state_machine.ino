///////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ALL NEW~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// CUT REASON KEY
// Ascent Timer ran out 0x00
// Termination altitude reached 0x01
// Slow ascent timer ran out 0x02
// In slow ascent, under threshold 0x03
// Float timer ran out 0x04
// Reached slow descent floor 0x05
// Slow descent timer ran out 0x06
// Out of bounds 0x07
// Master timer ran out 0x08
// Error - default state timers 0x09
//////////////////////////////////SUGGESTED STATE KEY//////////////////////////////////////////////
// Initialization 0x00
// Ascent 0x01
// Slow Ascent 0x02
// Float 0x03
// Slow Descent 0x04
// Descent 0x05
// Out of Bounds 0x06
// Master Timer Reached 0x07
// ERROR 0x08
/////////////////////////////////////CONTROL/////////////////////////////////////////////////////////
void Control() {
  // uses detData to determine what state it thinks we should be in
  // worst case states take priority, but every possible state is outputted as hex

  if (ascent_rate >= 1.905) {
    stateSuggest = ASCENT;
  }
  if (ascent_rate < 1.905 && ascent_rate >= 0.508) {
    stateSuggest = SLOW_ASCENT;
  }
  if (ascent_rate < 0.508 && ascent_rate > -0.508) {      //edit to fix the 0.00 ascent rate happening every 4 cycles implimented here

    //code to check if current calculated ascentRate is exactally zero, and if the ascentRate 4 cycles previous and 8 cycles previous behind are also zero.
    //if this is true that means that most likely there is a bad reading of altitude
//    if (last_ten_ascent_rates[9] == last_ten_ascent_rates[5] && last_ten_ascent_rates[9] == last_ten_ascent_rates[1]) { 
//      //since there is a known pattern of incorrect ascent rate calculations, dont change stateSuggest
//      return;
//    }
    
    stateSuggest = FLOAT;
  }
  if (ascent_rate <= -0.508 && ascent_rate > -1.905) {
    stateSuggest = SLOW_DESCENT;
  }
  if (ascent_rate <= -1.905 && ascent_rate > -10.0) {
    stateSuggest = DESCENT;
  }

  //adding parachute decent state to be used for the third resistor burn 
  if(ascent_rate <= -10.0){
    stateSuggest = FAST_DESCENT;
  }

  if (((lon > EASTERN_BOUNDARY) || (lon < WESTERN_BOUNDARY) || (latitude > NORTHERN_BOUNDARY) || (latitude < SOUTHERN_BOUNDARY)) && !((lon == 0) && (latitude == 0))) {
    boundsSuggest = OUT_OF_BOUNDS; //changed for GL196 code
  }

  if (altFeet < ALTITUDE_FLOOR) { //set to 5000 ft rn
    stateSuggest = INITIALIZATION;
    Serial.println("Suggested State: Initialization (or error or past timer if that comes next)");
  }

  if (currTimeS > 240 * 60) {
    stateSuggest = PAST_TIMER;
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////STATE/////////////////////////////////////////////////////////////////
String State() {
  // take in stateSuggest from control and switches into that state after a predetermined number of hits
  stateReturn = "Initialization"; //FOR TESTING PURPOSES
  switch (stateSuggest) {
    ////ASCENT////
    case ASCENT:

      Serial.println("Suggested State: Ascent");
      stateReturn = "Ascent"; //FOR TESTING PURPOSES

      if (currentState != ASCENT) { // criteria for entering Ascent functionality
        ascentCounter += 1; // increment ascent counter
        SAcounter = 0, floatCounter = 0, SDcounter = 0, descentCounter = 0, fastDescentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0; // reset all other state counters

        if (ascentCounter >= 30 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 30 consecutive state suggestions
          currentState = ASCENT;
          ascentStamp = millis();
        }
      }

      if (currentState == ASCENT) { // operations while in ascent
        Serial.println("Current State: Ascent");

        if (millis() - ascentStamp >= ASCENT_TIMER) { // if ascent timer is reached
          cutReasonA = 0x00;

        }

        if (altFeet > ALTITUDE_CEILING) { // if ceiling is reached
          cutReasonA = 0x01;
        }

      }

      break;

    ////SLOW ASCENT////
    case SLOW_ASCENT:

      Serial.println("Suggested State: Slow Ascent");
      stateReturn = "Slow Ascent"; //FOR TESTING PURPOSES

      if (currentState != SLOW_ASCENT) { // criteria for entering Slow Ascent functionality
        SAcounter += 1; // increment slow ascent counter
        ascentCounter = 0, floatCounter = 0, SDcounter = 0, descentCounter = 0, fastDescentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0; // reset all other state counters

        if (SAcounter >= 60 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 60 consecutive state suggestions
          currentState = SLOW_ASCENT;
          SAstamp = millis();
        }
      }

      if (currentState == SLOW_ASCENT) { // operations while in slow ascent
        Serial.println("Current State: Slow Ascent");
        if (altFeet > ALTITUDE_CEILING) { // if ceiling reached
          cutReasonA = 0x01;
        }
        if (millis() - SAstamp >= SA_TIMER) { // if slow ascent timer reached
          cutReasonA = 0x02;
        }

        if (altFeet < SA_FLOOR) { // cuts immediately under threshold
          cutReasonA = 0x03;
        }
      }

      break;

    ////FLOAT////
    case FLOAT:

      Serial.println("Suggested State: Float");
      stateReturn = "Float"; //FOR TESTING PURPOSES

      if (currentState != FLOAT) { // criteria for entering Float functionality
        floatCounter += 1; // increment float counter
        ascentCounter = 0, SAcounter = 0, SDcounter = 0, descentCounter = 0, fastDescentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0; // reset all other state counters

        FARcounter = 0;

        if (floatCounter >= 180 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 180 consecutive state suggestions
          currentState = FLOAT;
          floatStamp = millis();
        }
      }

      if (currentState == FLOAT) { // operations while in float

        Serial.println("Current State: Float");
        if (altFeet > ALTITUDE_CEILING) { // if ceiling is reached
          cutReasonA = 0x01;
        }

        if (millis() - floatStamp >= FLOAT_TIMER) { // if timer reached
          cutReasonA = 0x04;
          cutReasonB = 0x04;
        }
      }

      break;

    ////SLOW DESCENT////
    case SLOW_DESCENT:

      Serial.println("Suggested State: Slow Descent");
      stateReturn = "Slow Descent"; //FOR TESTING PURPOSES

      if (currentState != SLOW_DESCENT) { // criteria for entering Slow Descent functionality
        SDcounter += 1; // increment slow descent counter
        ascentCounter = 0, SAcounter = 0, floatCounter = 0, descentCounter = 0, fastDescentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0; // reset all other state counters

        if (SDcounter >= 30 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 30 consecutive state suggestions
          currentState = SLOW_DESCENT;
          SDstamp = millis();
        }
      }

      if (currentState == SLOW_DESCENT) { // operations while in slow descent
        Serial.println("Current State: Slow Descent");
        if (altFeet < SLOW_DESCENT_FLOOR) { // if floor is reached
          cutReasonA = 0x05;
          cutReasonB = 0x05;
        }

        else if (millis() - SDstamp >= SLOW_DESCENT_TIMER) { // if timer is reached
          cutReasonA = 0x06;
          cutReasonB = 0x06;
        }
      }

      break;


    ////DESCENT////
    case DESCENT:

      Serial.println("Suggested State: Descent");
      stateReturn = "Descent"; //FOR TESTING PURPOSES

      if (currentState != DESCENT) { // criteria for entering Descent functionality
        descentCounter += 1; // increment descent counter
        ascentCounter = 0, SAcounter = 0, floatCounter = 0, SDcounter = 0, fastDescentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0; // reset all other state counters

        if (descentCounter >= 30 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 30 consecutive state suggestions
          currentState = DESCENT;
          descentStamp = millis();
        }
      }

      if (currentState == DESCENT) { // operations while in descent
        Serial.println("Current State: Descent");
        if (millis() - descentStamp >= SLOW_DESCENT_TIMER) { // reuses SD timer as a backup
          cutReasonA = 0x06;
          cutReasonB = 0x06;
        }
      }

      break;

       ////FAST_DESCENT////
    case FAST_DESCENT:

      Serial.println("Suggested State: Fast Descent");
      stateReturn = "Fast_Descent"; //FOR TESTING PURPOSES

      if (currentState != FAST_DESCENT) { // criteria for entering Descent functionality
        fastDescentCounter += 1; // increment descent counter
        ascentCounter = 0, SAcounter = 0, floatCounter = 0, SDcounter = 0, descentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0, timerCounter = 0; // reset all other state counters

        if (fastDescentCounter >= 30 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 30 consecutive state suggestions
          currentState = FAST_DESCENT;
        }
      }

      break;


    ////OUT OF BOUNDARY////
    case OUT_OF_BOUNDS:  //for GL196 there is no out of bounds set currently to suggest State

      Serial.println("Suggested Bounds: Out of Bounds");
      stateReturn = "OOB"; //FOR TESTING PURPOSES

      if (currentState != OUT_OF_BOUNDS) { // criteria for entering Out of Boundary functionality
        boundCounter += 1; // increment out of boundary counter
        ascentCounter = 0, SAcounter = 0, floatCounter = 0, SDcounter = 0, descentCounter = 0;
        tempCounter = 0, battCounter = 0, timerCounter = 0; // reset all other state counters

        if (boundCounter >= 180 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 180 consecutive state suggestions
          currentState = OUT_OF_BOUNDS;
        }
      }

      if (currentState == OUT_OF_BOUNDS) { // operations while out of boundary
        Serial.println("Current State: Out of Bounds");
        cutReasonA = 0x07;
        cutReasonB = 0x07;
      }

      break;

    ////MASTER TIMER REACHED////
    case PAST_TIMER:

      Serial.println("Suggested State: Past Timer");
      stateReturn = "Past Timer"; //FOR TESTING PURPOSES

      if (currentState != PAST_TIMER) { // criteria for entering Master Timer Reached functionality
        timerCounter += 1; // increment ascent counter
        ascentCounter = 0, SAcounter = 0, floatCounter = 0, SDcounter = 0, descentCounter = 0;
        tempCounter = 0, battCounter = 0, boundCounter = 0; // reset all other state counters

        if (timerCounter >= 10) { // activates after 10 consecutive state suggestions, regardless of altitude
          currentState = PAST_TIMER;
        }
      }

      if (currentState == PAST_TIMER) { // operations after Master Timer reached
        Serial.println("Current State: Past Timer");
        cutReasonA = 0x08;
        if (timeCounter == 1) {
          timerStampCutA = millis();
          timeCounter++;
        }
        if (millis() - timerStampCutA >= SLOW_DESCENT_TIMER) { // wait to cut B (hopefully to get slow descent data)
          cutReasonB = 0x08;
        }
      }

      break;

    ////DEFAULT////
    default:

      // if initialization never triggers, moves to the next of the function where it cuts A then B

      if (currentState == INITIALIZATION) { // currentState is initialized as INITIALIZATION, no other states have been activated
        Serial.println("Current State: Initialization");
        if ((lon > EASTERN_BOUNDARY) || (lon < WESTERN_BOUNDARY) || (latitude > NORTHERN_BOUNDARY) || (latitude < SOUTHERN_BOUNDARY)) { //A warning check for flight boundaries
          Serial.println("WARNING: Flight is out of boundaries. Change boundaries or move into proper area.");
        }
        if (initCounter == 1) {
          defaultStamp = millis();
          initCounter++;
        }
        if ( (millis() - defaultStamp) >= (INITIALIZATION_TIME + ASCENT_TIMER) ) { // gives an extra time buffer to leave initialization

          cutReasonA = 0x09;
          if (initCounter2 == 1) {
            defaultStampCutA = millis();
            initCounter2++;
          }
          if (millis() - defaultStampCutA >= SLOW_DESCENT_TIMER) { // wait to cut B (hopefully to get slow descent data)
            cutReasonB = 0x09;
          }
        }

      }

      else {
        Serial.println("Current State: Error");
        stateReturn = "Error"; //FOR TESTING PURPOSES

        // if it's not in initialization, means it entered another state at some point, then everything stopped working and stateSuggest = ERROR_STATE now
        // should wait a while to see if it will enter a state again, then do the same cut strategy, hoping for some slow descent
        defaultStamp2 = millis();
        if ( (millis() - defaultStamp2) >= (DEFAULT_TIME + ASCENT_TIMER) ) { // gives an extra time buffer to leave default
          cutReasonA = 0x09;
          defaultStampCutA = millis();
          if (millis() - defaultStampCutA >= SLOW_DESCENT_TIMER) { // wait to cut B (hopefully to get slow descent data)
            cutReasonB = 0x09;
          }
        }
      }

      break;

  }

   if(boundsSuggest == OUT_OF_BOUNDS) {//for GL196 there is no out of bounds set currently to suggest State

      Serial.println("Suggested Bounds: Out of Bounds");
      boundsReturn = "OOB"; //FOR TESTING PURPOSES

      if (boundsCurrent != OUT_OF_BOUNDS) { // criteria for entering Out of Boundary functionality
        boundCounter += 1; // increment out of boundary counter

        if (boundCounter >= 180 && altFeet > ALTITUDE_FLOOR) { // doesn't activate below floor or before 180 consecutive state suggestions
          boundsCurrent = OUT_OF_BOUNDS;
        }
      } 

      if (currentState == OUT_OF_BOUNDS) { // operations while out of boundary
        Serial.println("Current State: Out of Bounds");
        cutReasonA = 0x07;
        cutReasonB = 0x07;
      }

   } else { //suggested state is in bounds
    boundsCounter = 0;
    boundsCurrent = IN_BOUNDS;
   }

  
  Serial.println();
  if (cutReasonA != 0x22 || cutReasonB != 0x22) {
    Serial.print("CUT REASON A: ");
    Serial.print(cutReasonA, HEX);
    Serial.print(", CUT REASON B: ");
    Serial.println(cutReasonB, HEX);
    Serial.println();
  }
  Serial.println(altFeet);
  Serial.println(ascent_rate);

  return stateReturn;
}
