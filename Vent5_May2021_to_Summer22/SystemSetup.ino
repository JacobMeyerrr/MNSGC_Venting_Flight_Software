// ************************************************SETUP FUNCTION******************************************************

void systemSetup()
{
  Serial.begin(9600); // temporary, probably don't use on flight b/c of the gps
  delay(5000);
  Serial.println("Setup Beginning... June 2022 Flight Code Being Run.");
  Serial.println("6/8/22 ETJ Edit of ^^^^^^^");

  // LED Setup
    ledSetup();
    
  // Setting up resistor-burners
    burnSetup();
    burn2Setup();

  // Heater Setup
    heaterSetup();

  //radio Setup
    //radioSetup();

  // Pressure Sensor Setup
    while(!baro.begin())
    {
      Serial.println("baro init failed!");
    }
    Serial.println("Barometer initialization complete");

    pressureToAltitudeSetup();
    
    
  // UBLOX-8 GPS (UbloxGPS Library)
    Serial.println("Beginning GPS Setup");
    Serial5.begin(UBLOX_BAUD);//starts communication to UBLOX
    gps.init();
    // change back to WHILE statement for flight!!!
    if(!gps.setAirborne()) // loop should terminate once it's set to airborne mode. The LEDs will pulse to show it's not set
    {
      Serial.println("Not Set to Airborne Mode");
      ledON1();
      ledON2();
      ledON3();
      delay(800);
      ledOFF1();
      ledOFF2();
      ledOFF3();
      delay(100);
    }
    Serial.println("Set to airborne mode");
    Serial.println("GPS Setup Ended");

  // Built-in Teensy SD Card
    sdSetup();

  // Servo/Flapper       <<<----------------------------- Adding funcitonality to record min and max values of Servo position 
    ventServo.attach(PWM_PIN);    // initialze servo
    closeVent(); //Open and close the vent to show we are up and running
    delay(1000);
    openVent();
    delay(1000);
    closeVent();
    delay(1000);
    openVent();
    delay(1000);
    closeVent();
    delay(500);
    
 
  Serial.println("Beginning!");
}
