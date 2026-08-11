// ****************************SD Card Functions*******************************

//// Built-in Teensy SD-Card Data Logger

void sdSetup(){
  pinMode(chipSelect,OUTPUT);
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present");
    for(int i=1; i<20; i++){
      digitalWrite(13,HIGH);
      delay(100);
      digitalWrite(13,LOW);
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
