// *********************************************** LED Pattern Function + LED Helper Functions **************************************************

// ********************** Main LED Function ***********************

// Flash the LED in the appropriate pattern, to indicate the state of the vent to any upward facing cameras below. Key is in the google drive
void led()
{
  if(gps.getAlt_feet() == 0){//This happens when the GPS is just getting a read or isn't working. 2 LEDs flash in a cycle, with 1 turning off.
    if(millis()%1500<500){
      ledON1();
      ledON2();
      ledOFF3();
    }else if(millis()%1500<1000 && millis()%1500>=500){
      ledOFF1();
      ledON2();
      ledON3();
    }else{
      ledON1();
      ledOFF2();
      ledON3();
    }
  }
  else if(Res1on||Res2on)
  {
    if(millis()%1500<500)
    {
      ledON1();
      ledOFF2();
      ledOFF3();
    }
    else if(millis()%1500<1000 && millis()%1500>=500)
    {
      ledON1();
      ledON2();
      ledOFF3();
    }
    else
    {
      ledON1();
      ledON2();
      ledON3();
    }
  }
  else if(terminationBegun || ((lon > EASTERN_BOUNDARY) || (lon < WESTERN_BOUNDARY) || (latitude > NORTHERN_BOUNDARY) || (latitude < SOUTHERN_BOUNDARY))){ //The balloon will be terminating or is out of bounds; Rapidly flash all LEDs
    if(millis()%200<100){
      ledON1();
      ledON2();
      ledON3();
    }else{
      ledOFF1();
      ledOFF2();
      ledOFF3();
    }
  } else
  if(AlreadyStartedPreVenting1 && !AlreadyFinishedPreVenting1){ //All LEDs are kept solid for the first pre-vent
    ledON1();
    ledON2();
    ledON3();
  } else
  if(AlreadyStartedPreventing2 && !AlreadyFinishedPreventing2){ //All LEDs slowly flash for the second pre-vent
    if(millis()%1600<800){
      ledON1();
      ledON2();
      ledON3();
    }else{
      ledOFF1();
      ledOFF2();
      ledOFF3();
    }
  } else
  if(stateSuggest == INITIALIZATION){ //Slowly flashes each LED in succession
    if(millis()%1500<500){
      ledON1();
      ledOFF2();
      ledOFF3();
    }else if(millis()%1500<1000 && millis()%1500>=500){
      ledOFF1();
      ledON2();
      ledOFF3();
    }else{
      ledOFF1();
      ledOFF2();
      ledON3();
    }
  } else
    if(stateSuggest == SLOW_ASCENT){ //Keeps LED 1 on, 2 and 3 turned off (Red)
    ledON1();
    ledOFF2();
    ledOFF3();
  } else
  if(stateSuggest == ASCENT){ //Keeps LED 1 and 2 on , and 3 turned off (Red / Yellow)
    ledON1();
    ledON2();
    ledOFF3();
  } else
  if(stateSuggest == FLOAT){ //Keeps just LED 3 on (Blue)
    ledOFF1();
    ledOFF2();
    ledON3();
  } else
  if(stateSuggest == SLOW_DESCENT){ //Keeps LED 2 and 3 on (Yellow / Blue)
    ledOFF1();
    ledON2();
    ledON3();
  } else
  if(stateSuggest == DESCENT){ //Keeps just LED 2 on (Yellow)
    ledOFF1();
    ledON2();
    ledOFF3();
  } 
}

// *******LED helper functions*******

//Setup the LED
void ledSetup()
{
  pinMode(23, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(17, OUTPUT);
  ledON1();
  ledON2();
  ledON3();
  delay(2000);
  ledOFF1();
  ledOFF2();
  ledOFF3();
}

// turn on LED 1 (currently pin 23)
void ledON1()
{
  digitalWrite(23,HIGH);
}

//turn on LED 2 (currently pin 22)
void ledON2()
{
  digitalWrite(22,HIGH);
}

//turn on LED 3 (currently pin 17)
void ledON3()
{
  digitalWrite(17,HIGH);
}

// turn off LED 1 (currently pin 23)
void ledOFF1()
{
  digitalWrite(23,LOW);
}

// turn off LED 2 (currently pin 22)
void ledOFF2()
{
  digitalWrite(22,LOW);
}

// turn off LED 3 (currently pin 17)
void ledOFF3()
{
  digitalWrite(17,LOW);
}
