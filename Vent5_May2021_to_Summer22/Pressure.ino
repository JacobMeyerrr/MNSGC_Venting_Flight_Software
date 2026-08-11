// ************************************************PRESSURE SENSOR FUNCTIONS******************************************************


void updateBaro()
{
  msPressure = baro.readPressure(); 
  msPressure = msPressure * 0.000145038;
}

void pressureToAltitudeSetup()
{
  float h1 = 36152.0;
  float h2 = 82345.0;
  float T1 = 59-.00356*h1;
  float T2 = -70;
  float T3 = -205.05 + .00164*h2;
  pressureBoundary1 = (2116 * pow(((T1+459.7)/518.6),5.256));
  pressureBoundary2 = (473.1*exp(1.73-.000048*h2));
  pressureBoundary3 = (51.97*pow(((T3 + 459.7)/389.98),-11.388));
}

void pressureToAltitude(){
  float pressurePSF = (msPressure*144);
  
  float altPres = -100.0;
  if (pressurePSF > pressureBoundary1){// altitude is less than 36,152 ft ASL
     altPres = (459.7+59-518.6*pow((pressurePSF/2116),(1/5.256)))/.00356;
  }
  else if (pressurePSF <= pressureBoundary1 && pressurePSF > pressureBoundary2){ // altitude is between 36,152 and 82,345 ft ASL
    altPres = (1.73-log(pressurePSF/473.1))/.000048;
  }
  else if (pressurePSF <= pressureBoundary2){// altitude is greater than 82,345 ft ASL
    altPres = (459.7-205.5-389.98*pow((pressurePSF/51.97),(1/-11.388)))/-.00164;
  } 
  pressureAltFeet = altPres;
}
