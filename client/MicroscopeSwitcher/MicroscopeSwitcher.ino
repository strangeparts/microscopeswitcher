/*
  Analog Input

  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED) connected to digital pin 13.
  The amount of time the LED will be on and off depends on the value obtained
  by analogRead().

  The circuit:
  - potentiometer
    center pin of the potentiometer to the analog input 0
    one side pin (either one) to ground
    the other side pin to +5V
  - LED
    anode (long leg) attached to digital output 13
    cathode (short leg) attached to ground

  - Note: because most Arduinos have a built-in LED attached to pin 13 on the
    board, the LED is optional.

  created by David Cuartielles
  modified 30 Aug 2011
  By Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInput
*/

#include "wificredentials.h"
#include <WiFi.h>

int sensorPin = A4;    // select the input pin for the potentiometer
// int onOffSwitchPin = 35;
int onOffSwitchPin = 22;
// int onOffLedPin = 33;
int onOffLedPin = 23;
int potPin = A6;
int ledPin = LED_BUILTIN;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor
int startingLightVal = 1500;
int thresholdOffset = 200;

const int httpPort = 5000;
const char* server = "192.168.0.136";

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(onOffSwitchPin, INPUT);
  pinMode(potPin, INPUT);
  pinMode(onOffLedPin, OUTPUT);
  Serial.begin(115200);
  delay(100);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  startingLightVal = get_samples(5, 200);
  delay(1000);
  Serial.print("startingLightVal=");
  Serial.println(startingLightVal);
}

int get_samples(int num, int interval) {
  int sum = 0;
  for (int i = 0; i < num; i++) {
    sum += analogRead(sensorPin);
    delay(interval);
  }
  return sum / num;
}

int current_view = 0;
void switchCamera(int new_view) {
  if (new_view == current_view) {
    return;
  }

  current_view = new_view;

  char* view_name = "topdown";
  if (current_view == 1) {
    view_name = "microscope";
  }
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(server, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/switchcamera";
  url += "?view=";
  url += view_name;
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
      if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
      }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

int getPot() {
  return analogRead(potPin) / 2;
}

const int MICROSCOPE = 1;
const int TOPDOWN = 0;

void loop() {
  if (digitalRead(onOffSwitchPin)) {
    digitalWrite(onOffLedPin, LOW);
    return;
  } else {
    digitalWrite(onOffLedPin, HIGH);
  }

  int threshold = startingLightVal - getPot();
  Serial.print("threshold=");
  Serial.println(threshold);
  
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  if (sensorValue < threshold) {
    digitalWrite(ledPin, HIGH);
    switchCamera(MICROSCOPE);
  } else {
    digitalWrite(ledPin, LOW);
    switchCamera(TOPDOWN);
  }
  delay(500);
}
