//Smart Tape Meter
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//LCD*********************************
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//encoder pins************************
const int aPIN = D5;
const int bPIN = D6;
const byte encoderType = 0;  // encoder with equal # of detents & pulses per rev
//const byte encoderType = 1;             // encoder with  pulses = detents/2. pick one, commment out the other
const int THRESH = (4 - (2 * encoderType));  // transitions needed to recognize a tick - type 0 = 4, type 1 = 2
const byte ZEERO = 0x80;                     // byte data type doesn't do negative
volatile int currentValue = 0;
float dist = 0.0;
float lastDist = 0.0;
byte lastState = 0;
int steps = 0;
byte AState = 0;
byte BState = 0;
byte State = 0;
int b = 0;
int c = 0;
int unit = 1;
int j = 1;

unsigned long oldTime = 0;

IPAddress IP;
ESP8266WebServer server(80);
const char* ssid = "VIMED";
const char* password = "VIMED123";

int btn = 0;
int lastBtn = 0;
void setup() {
  Serial.begin(115200);

  //display setup********************************************
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // Mengatur WiFi ----------------------------------------------------------
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  IP = WiFi.softAPIP();
  Serial.print("Alamat IP Access Point: ");
  Serial.println(IP);
  delay(100);
  server.on("/", HTTP_GET, handle_getDist);
  server.on("/reset", HTTP_GET, handle_reset);
  server.begin();
  Serial.println("HTTP server started");
  main_menu();
  delay(2000);

  //encoder pins setup**********************************************
  pinMode(aPIN, INPUT_PULLUP);
  pinMode(bPIN, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
}

void loop() {
  server.handleClient();
  btn = digitalRead(D7);
  dist = 0.2 * (float)steps;
  if ((millis() - oldTime) > 100) {
    oldTime = millis();
    display.clearDisplay();
    display.setCursor(5, 10);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.println(((String)dist));
    display.setCursor(90, 10);
    display.println(F("CM"));
    if (dist >= 50) dist = 0;
    display.display();
  }

  AState = digitalRead(bPIN);
  BState = digitalRead(aPIN) << 1;
  State = AState | BState;
  if (lastState != State) {
    switch (State) {
      case 0:
        if (lastState == 2) {
          steps++;
        } else if (lastState == 1) {
          steps--;
        }
        break;
      case 1:
        if (lastState == 0) {
          steps++;
        } else if (lastState == 3) {
          steps--;
        }
        break;
      case 2:
        if (lastState == 3) {
          steps++;
        } else if (lastState == 0) {
          steps--;
        }
        break;
      case 3:
        if (lastState == 1) {
          steps++;
        } else if (lastState == 2) {
          steps--;
        }
        break;
    }
  }
  b = lastState;
  lastState = State;
  if (b != State) {
    Serial.println(dist);
  }
  if (btn != lastBtn) {
    Serial.println(btn);
    lastBtn = btn;
  }
  if (btn == 0) {
    currentValue = 0;
  }
}

void handle_getDist() {
  Serial.println("Get Distance");
  server.send(200, "text/html", (String)dist);
}

void handle_reset() {
  steps = 0;
  Serial.println("Reset Distance");
  server.send(200, "text/plain", "0.00");
}
