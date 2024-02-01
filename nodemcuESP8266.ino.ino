#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = ""; // type your auth token
char ssid[] = "here";  // type your wifi name
char pass[] = "here";    // type your wifi password

BlynkTimer timer;

int tdsValue = 0;
float phValue = 0;

BLYNK_WRITE(V0){
  tdsValue = param.asInt();
}

BLYNK_WRITE(V1){
  phValue = param.asFloat();
}

void myTimer() {
  Blynk.virtualWrite(V0, tdsValue);
  Blynk.virtualWrite(V1, phValue);
}

void setup() {
  timer.setInterval(1000L, myTimer);
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

void loop() {
  if (Serial.available() >= 3) {
    tdsValue = Serial.parseInt();     
    Serial.read();                       
    phValue = Serial.parseFloat();  
    Blynk.virtualWrite(V0, tdsValue);
    Blynk.virtualWrite(V1, phValue);
  }
  Blynk.run();
  timer.run();  
}

