//Smart Tape Meter
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

//LCD*********************************
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const char* menuItems[] = { "Get Curve A", "Get Curve B", "Get Curve C", "Saved Data" };
const int numMenuItems = 4;
int selectedMenuItem = 0;

//encoder pins************************
const int aPIN = D5;
const int bPIN = D6;
const byte encoderType = 0;  // encoder with equal # of detents & pulses per rev
//const byte encoderType = 1;             // encoder with  pulses = detents/2. pick one, commment out the other
const int THRESH = (4 - (2 * encoderType));  // transitions needed to recognize a tick - type 0 = 4, type 1 = 2
const byte ZEERO = 0x80;                     // byte data type doesn't do negative
volatile int currentValue = 0;
float dist_a = 0.0;
float dist_b = 0.0;
float dist_c = 0.0;
float lastDist = 0.0;
byte lastState = 0;
int steps = 0;
byte AState = 0;
byte BState = 0;
byte State = 0;
uint8_t b = 0;
uint8_t c = 0;

unsigned long oldTime = 0;

IPAddress IP;
ESP8266WebServer server(80);
const char* ssid = "VIMED";
const char* password = "VIMED123";

uint8_t btn1 = 0;
uint8_t btn2 = 0;
String runningText = "VIMED";
uint8_t screen = 0;
uint8_t curve_menu = 0;
int curve_a = 0;
int curve_b = 0;
int curve_c = 0;
//EEPROM ******************************************8
uint8_t addr_a = 10;
uint8_t addr_b = 20;
uint8_t addr_c = 30;
uint8_t count_save = 0;
uint8_t count_back = 0;
float saved_a = 0;
float saved_b = 0;
float saved_c = 0;
uint8_t flag_saved = 0;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  //display setup********************************************
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();
  testscrolltext(runningText);

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


  //encoder pins setup**********************************************
  pinMode(aPIN, INPUT_PULLUP);
  pinMode(bPIN, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);
  screen = 0;
}

void loop() {
  server.handleClient();
  btn1 = digitalRead(D7);
  btn2 = digitalRead(D4);
  switch (screen) {
    case 0:
      homescreen();
      if (btn2 == 0) {
        delay(250);
        screen = 1;
      }
      break;
    case 1:
      curve_menu = main_menu();
      if (btn2 == LOW) {
        if (curve_menu == 0) screen = 2;
        else if (curve_menu == 1) screen = 3;
        else if (curve_menu == 2) screen = 4;
        else if (curve_menu == 3) screen = 5;
        else screen = 1;
      }
      break;
    case 2:
      curve_a = read_encoder();
      dist_a = 0.2 * (float)curve_a;
      screen_value(dist_a);
      if (btn2 == 0) {
        count_save++;
        delay(200);
        if (count_save > 3) {
          count_save = 0;
          steps = 0;
          screen_saved();
          EEPROM.put(addr_a, dist_a);
          EEPROM.commit();  // Commit the chan
          delay(2000);
          screen = 1;
        }
      }
      if (btn1 == LOW) {
        count_back++;
        if (count_back > 10) {
          if (dist_a != 0) {
            count_back = 0;
            count_save = 0;
            steps = 0;
            curve_a = 0;
          } else {
            count_back = 0;
            count_save = 0;
            curve_menu = 0;
            steps = 0;
            screen = 1;
          }
        }
      }
      break;
    case 3:
      curve_b = read_encoder();
      dist_b = 0.2 * (float)curve_b;
      screen_value(dist_b);
      if (btn2 == 0) {
        count_save++;
        delay(200);
        if (count_save > 3) {
          count_save = 0;
          steps = 0;
          screen_saved();
          EEPROM.put(addr_b, dist_b);
          EEPROM.commit();  // Commit the chan
          delay(2000);
          screen = 1;
        }
      }
      if (btn1 == LOW) {
        count_back++;
        if (count_back > 10) {
          if (dist_b != 0) {
            count_back = 0;
            count_save = 0;
            steps = 0;
            curve_b = 0;
          } else {
            count_back = 0;
            count_save = 0;
            curve_menu = 0;
            steps = 0;
            screen = 1;
          }
        }
      }
      break;
    case 4:
      curve_c = read_encoder();
      dist_c = 0.2 * (float)curve_c;
      screen_value(dist_c);
      if (btn2 == 0) {
        count_save++;
        delay(200);
        if (count_save > 3) {
          count_save = 0;
          steps = 0;
          screen_saved();
          EEPROM.put(addr_c, dist_c);
          EEPROM.commit();  // Commit the chan
          delay(2000);
          screen = 1;
        }
      }
      if (btn1 == LOW) {
        count_back++;
        if (count_back > 10) {
          if (dist_c != 0) {
            count_back = 0;
            count_save = 0;
            steps = 0;
            curve_c = 0;
          } else {
            count_back = 0;
            count_save = 0;
            curve_menu = 0;
            steps = 0;
            screen = 1;
          }
        }
      }
      break;
    case 5:
      if (flag_saved == 0) {
        EEPROM.get(addr_a, saved_a);
        EEPROM.get(addr_b, saved_b);
        EEPROM.get(addr_c, saved_c);
        screen_data(saved_a, saved_b, saved_c);
        flag_saved = 1;
      }
      if (btn1 == LOW) {
        flag_saved = 0;
        curve_menu = 0;
        screen = 0;
      }
      break;
  }
}

int read_encoder() {
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
  lastState = State;
  if (steps < 0) steps = 0;
  return steps;
}

void handle_getDist() {
  EEPROM.get(addr_a, saved_a);
  EEPROM.get(addr_b, saved_b);
  EEPROM.get(addr_c, saved_c);
  server.send(200, "text/plain", (String)saved_a + "-" + (String)saved_b + "-" + (String)saved_c);
}

void handle_reset() {
  steps = 0;
  EEPROM.put(addr_a, 0);
  EEPROM.put(addr_b, 0);
  EEPROM.put(addr_c, 0);
  EEPROM.commit();
  server.send(200, "text/plain", "0.00-0.00-0.00");
}
