#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

WiFiClient wifiClient;

#define MANUAL_SW 0
#define SPINDLE_STAT 2

#define LASER 3
#define POWER_STAT 1

const char *ssid = "ESP1CNC";
const char *psk = "dustCollector";
bool toggle = true, alternator = false;

void dustCollectorEvt(String evt);
void laserEvt(bool laserStatus);

void setup() {
  pinMode(SPINDLE_STAT, INPUT);
  pinMode(MANUAL_SW, INPUT);
  
  delay(1000);

  pinMode(LASER, FUNCTION_3);
  pinMode(LASER, OUTPUT);
  digitalWrite(LASER, LOW);

  pinMode(POWER_STAT, FUNCTION_3);
  pinMode(POWER_STAT, INPUT);

  delay(1000);

  ////Serial.begin(115200);
  ////while (!Serial);
  ////Serial.println();
  ////Serial.println("Configuring wifi station mode...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.begin(ssid,psk);
  int contconexion = 0;
  while (WiFi.status() != WL_CONNECTED && contconexion < 50) {
    contconexion++;
    delay(500);
    ////Serial.print(".");
  }
  if (contconexion < 50)
  {
    ////Serial.print("\nWifi connected, my ipAddress is: ");
    ////Serial.println(WiFi.localIP());
  }
  else
  {
    ////Serial.println("\nConnection error");
  }
}

void loop() {
  // If statement to expect wifi is connected before trying any resuqest:
  if((WiFi.status() == WL_CONNECTED) ) {
    //SPINDLE CONTROL
    if (digitalRead(SPINDLE_STAT) && toggle == false) {
      delay(2000);
      dustCollectorEvt("offcollector");
      delay(500);
      dustCollectorEvt("offcollector");
      toggle = true;
      laserEvt(true);
    }
    else if (digitalRead(SPINDLE_STAT) == LOW && toggle == true){
      for (int i = 0; i <= 3; i++) {
        delay(240); laserEvt(false);
        delay(240); laserEvt(true);
      }
      if (digitalRead(SPINDLE_STAT) == LOW){
        dustCollectorEvt("oncollector");
        toggle = false;
        laserEvt(false);
      }
    }
    //COMPLEMENTARY LASER CONTROL
    if(digitalRead(POWER_STAT)==HIGH)
      laserEvt(false);
    else if(digitalRead(POWER_STAT)==LOW && toggle)//if cnc ON and last time toggle was set to true last time
      laserEvt(true);
    //SWITCH CONTROL
    if (digitalRead(MANUAL_SW) == LOW) {
      while (digitalRead(MANUAL_SW)  == LOW );
      if(alternator) dustCollectorEvt("oncollector");
      else dustCollectorEvt("offcollector");
      alternator = !alternator;
    }
  }
}

void dustCollectorEvt(String evt) {
    HTTPClient http;
    ////Serial.print("[HTTP] begin...\n");
    // requesting required link 
    String endPoint = "http://192.168.4.1/" + evt;
    http.begin(wifiClient, endPoint);
    ////Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    ////Serial.print(httpCode);
    http.end();
}

void laserEvt(bool laserStatus){
  digitalWrite(LASER, laserStatus);
}