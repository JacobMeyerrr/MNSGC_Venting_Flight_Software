#define heaterSet 10
#define heaterReset 9
#define thermHeaterPin A20
String heaterStatus = "HEATER OFF";

/////////////// Thermistor constants //////////////////////
int analogResolutionBits = 14;
float adcMax = pow(2,analogResolutionBits)-1.0; // The maximum adc value given to the thermistor
float A = 0.001125308852122; // A, B, and C are constants used for a 10k resistor and 10k thermistor for the steinhart-hart equation
float B = 0.000234711863267;
float C = 0.000000085663516; 
float R1 = 10000; // 10k Ω resistor
float Tinv;
float adcVal;
float logR;
float T; // these three variables are used for the calculation from adc value to temperature
float currentTempC; // The current temperature in Celcius
float currentTempF; // The current temperature in Fahrenheit

float minimumTemp = 32;
float tolerance = 1;
float heaterTempValue;
float minimumTemperature = minimumTemp;
float maximumTemperature = (minimumTemp + tolerance);

bool notSetHeaterTime = false;
unsigned long heaterTimer = 0;


void updateThermistor(){
  analogReadResolution(analogResolutionBits); //Changes other analog values - servoPos?
  adcVal = analogRead(thermHeaterPin);
  logR = log(((adcMax/adcVal)-1)*R1);
  Tinv = A+B*logR+C*logR*logR*logR;
  T = 1/Tinv;
  currentTempC = T-273.15; // converting to celcius
  currentTempF = currentTempC*9/5+32;
  heaterTempValue = currentTempF;
}


void heaterSetup(){
  pinMode(heaterSet,OUTPUT);
  pinMode(heaterReset,OUTPUT);
  digitalWrite(heaterReset,HIGH);
  delay(10);
  digitalWrite(heaterReset,LOW);
  heaterStatus = "HEATER OFF";
}


void setHeaterState(){
//heater test for 60-70min after power on
  //if((millis()/1000) > 3600 && (millis()/1000) < 4200){
   // digitalWrite(heaterSet,HIGH);
    //delay(10);
    //digitalWrite(heaterSet,LOW);
    //heaterStatus = "HEATER ON TIMER TEST";
  //}
  //else if
  if(heaterTempValue <= minimumTemperature){ 
    digitalWrite(heaterSet,HIGH);
    if(!notSetHeaterTime)
    {
      notSetHeaterTime = true;
      heaterTimer = millis();     
    }
    if(heaterTimer + 10 <= millis() && notSetHeaterTime)
    {
      notSetHeaterTime = false;
      digitalWrite(heaterSet,LOW);
      heaterStatus = "HEATER ON";
    }  
  }
  else if(heaterTempValue >= maximumTemperature){ 
    digitalWrite(heaterReset,HIGH);
    if(!notSetHeaterTime)
    {
      notSetHeaterTime = true;
      heaterTimer = millis();     
    }
    if(heaterTimer + 10 <= millis() && notSetHeaterTime)
    {
      notSetHeaterTime = false;
      digitalWrite(heaterReset,LOW);
      heaterStatus = "HEATER OFF";
    } 
  }
}
