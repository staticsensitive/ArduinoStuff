/*
* Send Sensor Data to Thingspeak
* Requirement:
* Gizduino UNO SE
* DHT11 Temp & Humidity Sensor
* SIM900A GSM/GPRS Module
* Sim Card with Data Subscription
* Thingspeak account
* Channel must have 2 Fields
* 
* Libraries: https://github.com/adafruit/Adafruit_Sensor
*            https://github.com/adafruit/DHT-sensor-library 
*/

#include <SoftwareSerial.h>
#include "DHT.h"
SoftwareSerial GSM(2, 3);

/*Dont forget to change your API Key Here*/
char WriteAPI[] = "Change this to your API Write Key";

char temp_str[100];
int temp;
int humid;

unsigned long prevMillis = 0;

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
boolean ledState = false;

void setup() {
  
    GSM.begin(9600);
    Serial.begin(9600); 
    dht.begin();
    delay(5000);
    pinMode(13,OUTPUT);

    modemInitialize();
    
   
}

void loop() {

      if(millis() - prevMillis >= 5000){
        prevMillis =  millis();
         int t = dht.readTemperature();
         int h = dht.readHumidity();

         Serial.print("Temp:");
         Serial.print(t);
         Serial.print("Humidity:");
         Serial.println(h);
         
         sendToThingspeak(t,h);
               
         digitalWrite(13,HIGH);delay(100);
         digitalWrite(13,LOW);
      }

}


/*change APN to your appropriate network*/
void modemInitialize()
{
  Serial.println("Initializing Modem");
  
  if(sendAT("AT","OK",1000)){
    Serial.println("Modem Present");
  }else{
    Serial.println("Cannot Find Modem");
    while(1);
  }
  delay(1000);

  Serial.println("Setting modem Parameters");
  sendAT("AT+CMGF=1","OK",1000);
  delay(1000);
  sendAT("AT+CNMI=2,2,0,0,0","OK",1000);
  delay(1000);
  Serial.println("Setting GPRS Parameters");   
  sendAT("AT+SAPBR=3,1,\"contype\",\"GPRS\"","OK",5000);
  delay(1000);
  sendAT("AT+SAPBR=3,1,\"APN\",\"http.globe.com.ph\"","OK",5000);
  delay(1000);
  sendAT("AT+SAPBR=1,1","OK",8000);
  delay(1000);
  Serial.println("Setting HTTP Parameters");
  sendAT("AT+HTTPINIT","OK",1000);
  delay(1000);
  sendAT("AT+HTTPPARA=\"CID\",1","OK",1000);
  Serial.println("Modem Initialization Done");
  Serial.println("Ready to Send Data");
}



void sendToThingspeak (int field1,int field2)
{
   sprintf(temp_str,"AT+HTTPPARA=\"URL\",\"api.thingspeak.com/update?api_key=%s&field1=%d&field2=%d\"",WriteAPI,field1,field2);
 
   if(sendAT(temp_str,"OK",10000)){
    Serial.println("Sending OK");
   }
   delay(3000);
   sendAT("AT+HTTPACTION=0","OK",5000);
   Serial.println("Data Sent");
}

int sendAT(char* cmd, char* resp, unsigned int tout)
{
    int ctr=0;
    int ans=0;
    char buff[100];
    unsigned long prev;
    memset(buff, '\0', 100);
    
    delay(100);
    
    while( GSM.available() > 0) GSM.read(); 
    GSM.println(cmd);    
    ctr = 0;
    prev = millis();
    do{
        if(GSM.available() != 0){    
            buff[ctr] = GSM.read();
            ctr++;
             if (strstr(buff, resp) != NULL)    
            {
                ans = 1;
            }
        }
    }while((ans == 0) && ((millis() - prev) < tout));    
    return ans;
}
