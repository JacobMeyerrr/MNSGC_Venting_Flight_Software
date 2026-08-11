// ****************************************************************************** MAIN LOOP + STATE MACHINE **************************************************************

// System data collection and state machine
void systemUpdate()
{
  // Flash the LED appropriately
  led(); // flash the LED in the desired pattern

  // \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Updating Variables 'n' such \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
  //Thermistor / heater / baro updates
  updateThermistor();
  setHeaterState();
  updateBaro();
  pressureToAltitude();
  updateRadio();

  // BELOW is necessary if you want to be able to send serial commands (not critical for flight, but super handy for testing)**********
  if (Serial.available()) { // need to constantly check for serial bytes/commands (as opposed to only every 2 seconds) so that's why this isn't inside the "if(currTime-prevTime>interval)" loop.
    serialcommand = Serial.readStringUntil('\n');
    Serial.print("You typed: " );
    Serial.println(serialcommand);
  }
  // ABOVE is necessary if you want to be able to send serial commands (not critical for flight, but super handy for testing)**********

  // Below: Check for GPS data, update buffer if data available

  // New GPS Update
  gps.update(); // refresh the GPS so it gives you new data


  // obtain the current time (in milliseconds)
  unsigned long currTime = millis();

  //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Beginning the every-2s cycle   \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/


  // Serial.print("q3456yuj9hbvcsanlrty[jmhpoknv dsmwerltphj[knm iM OUTSIDE OF THE LOOP dsfghj,.iojkdghlnmb lvkcjxbzmfg,h;");
  if (currTime - prevTime > interval) // Interval is currently 2000 ms (or 2 seconds) as defined - this is the main loop/cycle
  {
    //\/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ updating timers //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    prevTime = millis(); //setting prevTime to the current time --> this is for all functions that need every 2 seconds. not only ascent rate
    currTimeS = currTime / 1000; // obtain the current time in seconds (convert from milliseconds to seconds)
    if (FlightHasBegun) { //getting flight timer to record how long the balloon is considered to be flying because of the pressure decrease
      flightTime = currTimeS - timeBeforeFlight;
    }

    gpsHR = gps.getHour() - 5; //obtain time for GPS timestamp in SD card if GPS lock is available. Need minus 5 because of a time zone issue
    gpsMIN = gps.getMinute();
    gpsSEC = gps.getSecond();

    // Data line to log to the CSV file on our SD card
    message = String(flightTime) + "," + String(currTimeS) + "," + String(currTime) + "," + String(gpsHR) + "," + String(gpsMIN) + "," + String(gpsSEC) + "," + String(heaterTempValue) + "," + String(msPressure, 4) + "," + String(pressureAltFeet) + "," + boundsSuggest + "," + boundsCurrent; // the beginning of the message to be sent to our SD card. the ","'s (commas) are inserted as placeholders here since those variables weren't tracked/logged for this flight (see header to see which variables)

    //\/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ checking flapper state //\/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    Serial.println("Checking flapper State: " + flapperState);
    if (flapperState.equals("Open")) { //This checks to see if the vent needs to be "jiggled" to fix servo position in the case that it's unaligned
      openVent();
    } else {
      closeVent();
    }

    //\/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ updating GPS variables //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    // *******If GPS Lock is obtained, log that data (if statement), otherwise we resort to using the pressure sensor, which can only reliably track altitude to 60k feet************
    latitude = gps.getLat(); // obtain the latitude from the GPS
    lon = gps.getLon(); // obtain the longitude from the GPS
    altFeet = gps.getAlt_feet(); // obtain the altitude (in feet) from the GPS
    numSats = gps.getSats(); // obtain the number of satellites from the gps


    //\/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Calculating Ascent Rates  //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    Serial.println("Calculating gps_ascent_rate, currTime: " + String(currTime) + "\tprev_time2: " + String(prev_time2) + "\tRunTime: " + String(currTime - prev_time2));
    gps_ascent_rate = 0.3048 * (((altFeet - prev_Control_Altitude) / (currTime - prev_time2)) * 1000); // calculate ascent rate and convert it to m/s
    press_ascent_rate = 0.3048 * (((pressureAltFeet - prev_Press_Altitude) / (currTime - prev_time2)) * 1000); // calculate ascent rate and convert it to m/s


    if (GPS_LOCK == true) //if GPS lock is true then use GPS ascent rate calcultaed above, if not use pressure ascent rate for average ascent rate.
    {
      if (abs(altFeet - prev_Control_Altitude) > 1000) //if there is more than 1000ft of differance in the altitude hits claim bad hit
      {
        Serial.println("Bad GPS hit!");
        altFeet = prev_Control_Altitude + ascent_rate / 0.3048 * (currTime - prev_time2) / 1000;
      }

      ascent_rate = gps_ascent_rate;

    }
    else // This else statement is only initiated if the GPS doesn't have a lock. We now resort to pressure sensor backup
    {
      // obtain the altitude (in feet) from the pressure sensor - yeah not set to altFeet

      ascent_rate = press_ascent_rate;

      //since no GPS hit setting numSats equal to zero
      numSats = 0;

    }

    //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Adding new ascent rate into list of past 10 and calculating new average ascent rate //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/

    avg_ascent_rate = 0; // reset before/after every loop, then calculated in the following for loop (averaged over the last 10 hits--or 20 seconds worth of data--to reduce noise)

    for (int i = 0; i < 20; i++) // for loop to sum the ascent rates.
    {
      if (i < 19)
      {
        last_twenty_ascent_rates[i] = last_twenty_ascent_rates[i + 1];
        if (last_twenty_ascent_rates[i] != 0)
        {
          avg_ascent_rate += last_twenty_ascent_rates[i];

        }
      }
      if (i == 19)
      {
        last_twenty_ascent_rates[i] = ascent_rate;
        if (last_twenty_ascent_rates[i] != 0)
        {
          avg_ascent_rate += last_twenty_ascent_rates[i];

        }
      }
    }
    avg_ascent_rate = avg_ascent_rate / 20; // sumed up the last 10 ascent rates now divide by 10 to obtain the average ascent rate for this cycle

    // *****Below is printing to the serial monitor, convinent if you're testing with the serial cable*****
    Serial.println("Average Ascent Rate (below) is:"); // printing to the serial monitor, convinent if you're testing with the serial cable
    Serial.println(avg_ascent_rate); // printing to the serial monitor, convinent if you're testing with the serial cable
    Serial.println("Average Ascent Rate Printed Above."); // printing to the serial monitor, convinent if you're testing with the serial cable
    Serial.println("Last Ten Ascent Rates (below) are:"); // printing to the serial monitor, convinent if you're testing with the serial cable
    for (int i = 0; i < 19; i++) {
      Serial.print(String(last_twenty_ascent_rates[i]) + ",");
    }
    Serial.println(String(last_twenty_ascent_rates[19]));
    //Serial.println(String(last_ten_ascent_rates[0]) + "," + String(last_ten_ascent_rates[1]) + "," + String(last_ten_ascent_rates[2]) + "," + String(last_ten_ascent_rates[3]) + "," + String(last_ten_ascent_rates[4]) + "," + String(last_ten_ascent_rates[5]) + "," + String(last_ten_ascent_rates[6]) + "," + String(last_ten_ascent_rates[7]) + "," + String(last_ten_ascent_rates[8]) + "," + String(last_ten_ascent_rates[9]) + ",");
    Serial.println("Last Ten Ascent Rates Printed Above."); // printing to the serial monitor, convinent if you're testing with the serial cable
    // *****Above is printing to the serial monitor, convinent if you're testing with the serial cable*****

    //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ Adding GPS data to the 'message' string to put onto the SD card //\/ \/ \/ \/ \/ \/ \/ \/ \/ \/
    prev_time2 = millis();
    prev_Press_Altitude = pressureAltFeet;
    prev_Control_Altitude = altFeet; //prev_Control_Altitude is set to the pressure altitude
    GPSdata = "," + String(latitude, 4) + "," + String(lon, 4) + ","  + String(altFeet) + "," + String(numSats) + "," + String(ascent_rate) + "," + String(avg_ascent_rate) + "," + String(gps_ascent_rate) + "," + String(press_ascent_rate);
    message += GPSdata; // add that GPS data string to the overall SD log message


    if (gps.getAlt_feet() == 0) {
      GPS_LOCK = false;
      Serial.println("GPS LOCKED IS FALSE FALSE FALSE");
    }
    else {
      GPS_LOCK = true;
      Serial.println("GPS LOCKED IS TRUE TRUE TRUE");
    }

    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************
    // *****************State Determination ("Worst-case" to "Best Case" Order of Operations)***************

    // Start Flight Timer once 5000 feet threshold is surpassed (for the backup flight cutter)
    if (GPS_LOCK == true)
    {
      if (altFeet > 5000 && fivekFeetReached == false) // 5000 feet is the threshold to make sure that it doesn't prematurely vent or terminate when it's on the ground
      { // basically this if statement is only called the first moment after you cross 5000 feet for the first time and
        // it sets the flight status as having begun which starts the backup flight timer and allows you to start running through termination and vent logic.
        // This function sets the initial values of "ascent_rate," "avg_ascent_rate," and "last_twenty_ascent_rates" to 5 m/s so that it avoids accidentally tripping the termination logic early
        fivekFeetReached = true;
        timeSince5kFeetS = currTimeS;
        ascent_rate = 5;
        for (int i = 0; i < 20; i++) {
          last_twenty_ascent_rates[i] = 5;
        }
       avg_ascent_rate = 5;
      }
    }
    else if (GPS_LOCK == false)
    {
      if (pressureAltFeet > 5000 && fivekFeetReached == false) // 5000 feet is the threshold to make sure that it doesn't prematurely vent or terminate when it's on the ground
      { // basically this if statement is only called the first moment after you cross 5000 feet for the first time and
        // it sets the flight status as having begun which starts the backup flight timer and allows you to start running through termination and vent logic.
        // This function sets the initial values of "ascent_rate," "avg_ascent_rate," and "last_twenty_ascent_rates" to 5 m/s so that it avoids accidentally tripping the termination logic early
        timeSince5kFeetS = currTimeS;
        fivekFeetReached = true;
        ascent_rate = 5;
        for (int i = 0; i < 20; i++) {
          last_twenty_ascent_rates[i] = 5;
        }
       avg_ascent_rate = 5;
      }
    }
    // to be added: to use the pressure drop when flight begins to estiblish an more accurate flight timer
    if (FlightHasBegun == false) {

      if ((prevPress - msPressure) >=  0.01) {
        pressCount++;
      }
      else {
        pressCount = 0;
      }
      prevPress = msPressure;
      if (pressCount >= 10) { //changed this to 10 because then you need 20s of consistent ascent till it starts.
        FlightHasBegun = true;
        timeBeforeFlight = currTimeS;
      }
    }
    // Simply print to the serial monitor whether the flight has begun (0 or 1 boolean) and, if so, how much time since then
    Serial.println("FlightHasBegun and timeSince5kFeetS");
    Serial.println(FlightHasBegun);
    Serial.println(String(timeSince5kFeetS));


    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************
    //**************************************************************TERMINATION LOGIC BEGINS**************************************************************

    // *****Ultimate Emergency Timer Cut in case nothing burned or the vent didn't open for whatever reason before*******
    if (currTimeS > terminateTimeS) // Master Backup timer (currently cuts 14,400 seconds (4 hours) after turning on in case all else fails)
    {
      terminate();
      Serial.println("Released because of backup master timer");
      cutterState = "Released because of backup master timer";
    }

    // *********Backup Flight Cutter*************
    if (FlightHasBegun == true)
    {
      if (currTimeS - timeSince5kFeetS > 220 * 60) // Sort of redundant with the master timer, currently cuts 13,200 seconds (3 hours, 40 minutes) after the flight officially starts
      {
        terminate();
        Serial.println("Released because of backup flight timer");
        cutterState = "Released because of backup flight timer";
      }

      //this currently will never be called as of GL196 edit because currentState will never be out of bounds
      if (boundsCurrent == OUT_OF_BOUNDS ) //Check to terminate when the balloon has crossed the coordinate boundaries
      {
        terminate();
        cutterState = "Realeased because balloon crossed flight boundaries";
      }

      if ((preVentingTimeS1 + 10800 < currTimeS) && !AlreadyStartedPreVenting1) //Terminates if the flight has been in air for 3 hours and not hit the first pre-vent.
      {
        terminate();
        cutterState = "Released because it took over 3 hours to reach the first pre-vent from the flight start";
      }
      if ((preVentingTimeS1 + 3600 < currTimeS) && AlreadyFinishedPreVenting1 && !AlreadyStartedPreventing2) //Terminates when the balloon has hit pre-vent 1, but it has been an hour and not hit vent 2
      {
        terminate();
        cutterState = "Released because it took over an hour to reach the second pre-vent from the first pre-vent";
      }
      if ((preVentingTimeS2 + 3600 < currTimeS) && AlreadyFinishedPreventing2 && !AlreadyStartedBigVent) //Terminates when the balloon has hit pre-vent 2, but it has been an hour and not reached the final opening
      {
        terminate();
        cutterState = "Released because it took over an hour to reach the final vent opening from the second pre-vent";
      }
      if ((currentState == DESCENT || currentState == SLOW_DESCENT) && !AlreadyStartedVentToFloat) { //If the balloon is coming down and has not reached the final opening, wait 30 minutes, then cut.
        if (!descentTimerStarted) {
          descentTimerStarted = true;
          descentTimer = currTimeS;
        }
        if (descentTimer + 3600 < currTimeS) { //hour seems a bit long, float timer next to this for consistency? two false descent hour apart would terminate
          terminate();
          cutterState = "Released because balloon started coming down before final vent";
        }
      }
      if (altFeet < 4500 && altFeet > 3500 && (currentState == ASCENT || currentState == SLOW_ASCENT)) { //CHECK OVER THIS !!! If the balloon has dipped below 4500 feet after reaching 5000 feet to start the flight, we assume the balloon is in float along the initialization altitude
        terminate();
        cutterState = "Released because balloon dropped below 4500 feet after reaching 5000 feet";
      }
      if ((currentState == FLOAT || floatTimerStarted)) { //checks to see if balloon is floating after flight has begun, if floating for 60min then terminate
        if (!floatTimerStarted) {
          floatTimerStarted = true;
          floatTimer = currTimeS;
        }
        // backup termination logic for when the balloon has been in float for too long
        if (floatTimer + 3600 < currTimeS) {
          terminate();
          cutterState = "Released because balloon started floating before final vent";
        }
      }

      if (avg_ascent_rate <= 0.4 && !backupFloatTimerStarted) {
        backupFloatTimerStarted = true;
        backupFloatTimer = currTimeS;
      }
    }



    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************
    //**************************************************************TERMINATION LOGIC ENDS**************************************************************

    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************
    //**************************************************************VENTING LOGIC BEGINS**************************************************************

    // ****INSERT ALTITUE GPS-BASED AND TIMER-BASED VENTING DECISIONS HERE****

    // Regular State Machine
    if (GPS_LOCK == true) // if the GPS has had a hit in the last minute or so, assume that its working (this if statement is the primary (GPS-based) state machine logic
    {
      Serial.println("GPS Lock = " + String(GPS_LOCK));
      Serial.println("Going through regular state machine logic (GPS working)");
      Control();
      suggestedState = State();
      //////////////////////////////////////////////////////////////////// Flight Procedure //////////////////////////////////////////////////////////////////////////////

      if (altFeet > preVentAlt1) {
        PreVenting1(currTimeS, avg_ascent_rate);
      }

      if (altFeet > 70000) {
        VentToFloat(currTimeS, avg_ascent_rate);
      }
      if (altFeet > terminateAlt) {
        if (!terminationBegun) {
          cutterState = "Released because balloon is above or at termination altitude (90,000ft)";
        }
        Res1Burned = false;
        Res2Burned = false;
        terminate();
      }
      if (backupFloatTimer + 1200 < currTimeS) {
        if (!terminationBegun) {
          cutterState = "Released because balloon the avg ascent rate reached 0.4 m/s 20 mins ago";
        }
        Res1Burned = false;
        Res2Burned = false;
        terminate();
      }

      //////////////////////////////////////////////////////////////////// Flight Procedure //////////////////////////////////////////////////////////////////////////////


    }
    else // If GPS isn't working after one minute or so, resort to a  timer-based opening (This else statement is the backup logic, in the case that the GPS hasn't given you a hit for a few minutes)
    {
      Serial.println("Going through backup state machine logic");
      Control();
      suggestedState = State();

      if (pressureAltFeet > preVentAlt1) {
        PreVenting1(currTimeS, avg_ascent_rate);
      }

      if (pressureAltFeet > 70000) {
        VentToFloat(currTimeS, avg_ascent_rate);
      }
      if (pressureAltFeet > terminateAlt) {
        if (!terminationBegun) {
          cutterState = "Released because balloon is above or at termination altitude (90,000ft)";
        }
        terminate();
      }
      if (backupFloatTimer + (20 * 60) < currTimeS) {
        if (!terminationBegun) {
          cutterState = "Released because balloon the avg ascent rate reached 0.4 m/s 20 mins ago";
        }
        terminate();
      }
    }
    // *Note that all of the "Serial.println()"'s that are here are simply to tell someone what's going on if they are plugged into the Teensy and watching the serial monitor



    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************
    //**************************************************************VENTING LOGIC ENDS**************************************************************

    // BELOW IS FOR USE WITH THE SERIAL MONITOR ONLY**********************NOT FLIGHT-CRITICAL**********************

    if (serialcommand == "OPEN")
    {
      Serial.println("Testing");
      Serial.println("Vent open");
      ventReason = "Serial monitor test";
      openVent();
      serialcommand == "";
    }
    if (serialcommand == "CLOSE")
    {
      Serial.println("Vent closed");
      ventReason = "Closed";
      Serial.println("About to run closeVent() because of serial command");
      closeVent();
      serialcommand == "";
    }
    if (serialcommand == "BURN")
    {
      burnResistor();
      cutterState = "Burned";
      serialcommand == "";
    }
    if (serialcommand == "SBURN")
    {
      cutterState = "Stopped Burning";
      stopBurn();
      serialcommand == "";
    }
    if (serialcommand == "BURN2")
    {
      burnResistor2();
      cutterState = "Burned2";
      serialcommand == "";
    }
    if (serialcommand == "SBURN2")
    {
      cutterState = "Stopped Burning 2";
      stopBurn2();
      serialcommand == "";
    }
    if (serialcommand == "LEDON1")
    {
      digitalWrite(23, HIGH);
      serialcommand == "";
    }
    if (serialcommand == "LEDOFF1")
    {
      digitalWrite(23, LOW);
      serialcommand == "";
    }
    if (serialcommand == "LEDON2")
    {
      digitalWrite(22, HIGH);
      serialcommand == "";
    }
    if (serialcommand == "LEDOFF2")
    {
      digitalWrite(225, LOW);
      serialcommand == "";
    }
    if (serialcommand == "LEDON3")
    {
      digitalWrite(17, HIGH);
      serialcommand == "";
    }
    if (serialcommand == "LEDOFF3")
    {
      digitalWrite(17, LOW);
      serialcommand == "";
    }
    // ABOVE IS FOR USE WITH THE SERIAL MONITOR ONLY**********************NOT FLIGHT-CRITICAL**********************

    // Finally, add the last few variables to the message string, print out the data/message to the serial monitor, and log the message string to the SD Card
    radioData = heaterStatus + ", " + flapperState + ", " + String(servoPos()) + ", " + String(cutterState) + ", " + stateSuggest + ", " + currentState + ", " + ventReason;
    message += ", " + radioData;
    //BELOW IS FOR TESTING
    Serial.println(header);
    //ABOVE IS FOR TESTING
    Serial.println(message);
    Serial.println("");
    // SD Card
    if (true) {
      logData(message);                                        //close file afterward to ensure data is saved properly
    }
  } //if statement that checks if it has been 2 seconds
}
