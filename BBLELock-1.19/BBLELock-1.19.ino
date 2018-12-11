//
// Better BLE Lock v1.19
// SEC 465 IOT and Security
// Greg Foulds December 2018
// gfoulds001@my.wilmu.edu  &&  greg@foulds.org
//

#include <SPI.h>
#include "Adafruit_BLE_UART.h"
#define LOCK_PIN 6
#define RED_LED_PIN 4
#define GREEN_LED_PIN 5
#define BLE_REQ 10
#define BLE_RDY 2
#define BLE_RST 9
Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(BLE_REQ, BLE_RDY, BLE_RST);
long randNumber;
boolean ignore = false;

void setup(void)
{ 
  Serial.begin(9600);
  while(!Serial); 
  Serial.println(F("Better BLELock"));
  Serial.print("SKETCH FILE:   ");
  Serial.println(__FILE__);
  Serial.print("UPLOADED:   ");
  Serial.println(__DATE__);

  BTLEserial.setDeviceName("BLELock"); 
  BTLEserial.begin();
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW); 
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void loop()
{
  // Poll the nrf8001
  BTLEserial.pollACI();

  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(" ");
        Serial.println(F("***** NO CONNECTION  --  Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
        Serial.println(" ");
        Serial.println(F("***** APP CONNECTION DETECTED"));
        Serial.println(" ");
        ignore = false;
        randNumber = random(100000, 500000); // create random pin
        Serial.print("Generating random pin: ");
        Serial.println(randNumber);Serial.println(" ");
        Serial.print("Generating Challenge = ");
        Serial.println(randNumber*2);

// start send challenge
        String numberString = String(randNumber*2); // use super secret x2 encryption (hahaha)

        // convert string to bytes.  20 max.
        uint8_t sendbuffer[20];
        numberString.getBytes(sendbuffer, 20);
        char sendbuffersize = min(20, numberString.length());

        Serial.println(F("\n=====> Sending Challenge =====>")); 
        
        // write the data
        BTLEserial.write(sendbuffer, sendbuffersize);
      } 
      if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("\n\n***** Disconnected or advertising timed out"));
      }
    // OK set the last status change to this one
    laststatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {
    // Lets see if there's any data for us!
    if (BTLEserial.available()) {
      Serial.println(" ");
      Serial.println(F("<===== READING RESPONSE FROM APP <======"));
    }
    // OK while we still have something to read, get a character and print it out
    while (BTLEserial.available()) {
      String Str3;
      for (int i = 0; i < 6; i++){
        char c = BTLEserial.read();
        Str3+=(c);
      } 
    String num = Str3;
    int ii, len;
    long result=0;
 
    result = (num.toInt()); // convert string to int
    if (!ignore){
      if (result==randNumber){
        Serial.println(" ");
        Serial.println("PIN ACCEPTED  --  UNLOCKING FOR FIVE SECONDS");
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(LOCK_PIN, HIGH);
        delay(5000);
        Serial.println(" ");
        Serial.println("*RELOCKING*");
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(LOCK_PIN, LOW);
      }
      else
      {
        Serial.println(" ");
        Serial.println("INVALID PIN  --  IGNORING ALL FUTURE ATTEMPTS FOR THIS SESSION!");
        for (int j = 0; j < 10; j++){
          digitalWrite(RED_LED_PIN, HIGH);
          delay(300);
          digitalWrite(RED_LED_PIN, LOW);
          delay(300);
          ignore=true;
        }
      }
    }
  }
}
}
