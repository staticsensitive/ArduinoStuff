/*Calibration and Reading of Analog pH Sensor
 * 
 * Hardware Used:
 * 
 * Gizduino UNO-SE 
 * https://www.e-gizmo.net/oc/index.php?route=product/product&product_id=1405
 * 
 * DFRobot ph Meter V1.1
 * https://wiki.dfrobot.com/PH_meter_SKU__SEN0161_
 * 
 * Can also be used with other generic analog pH sensor kit
 *
 * Written by: Richard Myrick T. Arellaga
 * Urdaneta City University
 * 
 * Released for educational purposes only
 * No Warranty Given
*/
#include <EEPROM.h>

int EEPROMADD = 0x00 ;

#define ReceivedBufferLength 10
char   cmdReceivedBuffer[ReceivedBufferLength];  //store the Serial CMD
byte   cmdReceivedBufferIndex;

float phValue;
float acidVoltage;
float neutralVoltage;
float voltage;

int buf[10];
int temp;
unsigned long int avgValue;

float temp7,temp4;
float m,b;

float saved7,saved4,savedm,savedb;

#define SENSOR_PIN A0

float myphReading = 0.0;
unsigned long readmillis = 0;

bool calibrating = false;


void setup() {
  Serial.begin(9600);
  
  Serial.println("Checking Saved Settings");
  
  checkCalibration();
 
}

void loop() {

    if(millis() - readmillis > 1000 && !calibrating){
      readmillis = millis();
      myphReading = readPh();
      Serial.print("pH Reading:");
      Serial.println(myphReading);
    }

    if(serialDataAvailable() > 0){
      calibratePH(cmdParse());
    }
}

boolean serialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while(Serial.available()>0){
        if(millis() - cmdReceivedTimeOut > 500U){
            cmdReceivedBufferIndex = 0;
            memset(cmdReceivedBuffer,0,(ReceivedBufferLength));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex==ReceivedBufferLength-1){
            cmdReceivedBufferIndex = 0;
            strupr(cmdReceivedBuffer);
            return true;
        }else{
            cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
            cmdReceivedBufferIndex++;
        }
    }
    return false;
}


byte cmdParse (){
  byte idx = 0;
  if(strstr(cmdReceivedBuffer,"CALIB") != NULL){
    idx = 1;
  }else if(strstr(cmdReceivedBuffer,"PH7") != NULL){
    idx = 2;
  }else if(strstr(cmdReceivedBuffer,"PH4") != NULL){
     idx = 3;
  }else if(strstr(cmdReceivedBuffer,"EXIT") != NULL){
    idx = 4;
  }
  return idx;
}

void calibratePH (byte mode){
  /*Calibration is done using 2 points using the linear equation
  y=mx+b to find for the ph value */

  switch(mode){
     case 0: break;
     case 1: 
        calibrating = true;
        Serial.println(F("pH Sensor Calibration Mode"));
        Serial.println(F("Dip probe to pH 7 Buffer Solution"));
        Serial.println(F("Send PH7 to Continue"));
        break;
     case 2:
        Serial.println(F("pH7 Buffer Solution Reading"));
        Serial.println(F("Reading sensor...."));
        delay(30000);
        temp7 = readSensor();
        Serial.print("Sensor Voltage:");
        Serial.print(temp7);
        Serial.println();
        delay(1000);
        Serial.println(F("Done Reading.."));
        Serial.println(F("Clean with Distilled Water and Dip to ph4 Solution"));
        Serial.println(F("Send PH4 to Continue"));
        break;
     case 3:   
        Serial.println(F("pH4 Buffer Solution Reading"));
        Serial.println(F("Reading sensor...."));
        delay(30000);
        temp4 = readSensor();
        Serial.print("Sensor Voltage:");
        Serial.print(temp4);        
        delay(1000);
        Serial.println(F("Done Reading.."));
        Serial.println(F("Clean probe with Distilled Water"));
        Serial.println(F("Send EXIT to Calculate and save Settings"));
        break;
     case 4:
        Serial.println("Calculating Slope and Intercept");
        m = (7.0-4.0)/(temp7 - temp4);
        b = 7.0-(temp7 * m);
        Serial.print("Slope:");
        Serial.print(m);
        Serial.print("  ");
        Serial.print("Intercept:");
        Serial.print(b);
        Serial.println(); Serial.println();
        
        EEPROM.put(EEPROMADD,temp7);
        EEPROMADD += sizeof(float);
        EEPROM.put(EEPROMADD,temp4);
        EEPROMADD += sizeof(float);
        EEPROM.put(EEPROMADD,m);
        EEPROMADD += sizeof(float);
        EEPROM.put(EEPROMADD,b);
        
        Serial.println("Settings Saved!");       
        Serial.println("Board is Ready to Read pH");
        calibrating = false;
  }
}

float readSensor()
{
      for(int i=0; i<10; i++){
        buf[i] = analogRead(SENSOR_PIN);
        delay(10);
     }

      for(int i=0;i<9;i++)
        {
          for(int j=i+1;j<10;j++)
            {
              if(buf[i]>buf[j])
                {
                  temp=buf[i];
                  buf[i]=buf[j];
                  buf[j]=temp;
                }
            }
          }

          avgValue = 0;
          for(int i=2; i<8; i++){
            avgValue += buf[i];
          }

      float phVol = (float)avgValue * (5.0)/1023.0/6;
    
    return phVol;
}

void checkCalibration()
{
   if(EEPROM.read(EEPROMADD) == 0xFF && EEPROM.read(EEPROMADD+1) == 0xFF && EEPROM.read(EEPROMADD+2) == 0xFF && EEPROM.read(EEPROMADD+3) == 0xFF){
     Serial.println("Board not Calibrated, Please run Calibration from Serial Terminal");
   }else{
     Serial.println("Getting Settings");
     
     EEPROM.get(EEPROMADD,saved7);
     EEPROMADD += sizeof(float);
     EEPROM.get(EEPROMADD,saved4);
     EEPROMADD += sizeof(float);
     EEPROM.get(EEPROMADD,savedm);
     EEPROMADD += sizeof(float);
     EEPROM.get(EEPROMADD,savedb);

    Serial.print("Slope:"); Serial.print(savedm);
    Serial.print("Intercept"); Serial.println(savedb);
     
   }
}

float readPh()
{
   float senVal = readSensor();
   float ph = savedm * senVal + savedb;

   return ph;
}
