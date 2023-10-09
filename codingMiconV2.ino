//Smart Tape Meter
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//LCD*********************************
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//encoder pins************************
const int aPIN = D5;
const int bPIN = D6;
const byte encoderType = 0;             // encoder with equal # of detents & pulses per rev
//const byte encoderType = 1;             // encoder with  pulses = detents/2. pick one, commment out the other
const int THRESH = (4 - (2 * encoderType)); // transitions needed to recognize a tick - type 0 = 4, type 1 = 2
const byte ZEERO = 0x80;                // byte data type doesn't do negative
volatile int currentValue = 0;
float dist = 0.0;
float lastDist = 0.0;
float P = 7;//Perimeter
float N = 20;//Encoder steps per revelation
byte CURRENT;                           // the current state of the switches
byte INDEX = 15;                        // Index into lookup state table
byte TOTAL = 0;
// Encoder state table - there are 16 possible transitions between interrupts
int ENCTABLE[]  = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};

int count = 0;
unsigned long oldTime = 0;

/* Put IP Address details */
IPAddress IP;
ESP8266WebServer server(80);
const char* ssid     = "VIMED";
const char* password = "VIMED123";

int btn=0;
int lastBtn=0;
void setup() {
  Serial.begin(115200);

  //display setup********************************************
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
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
  attachInterrupt(digitalPinToInterrupt(aPIN), Encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(bPIN), Encoder, CHANGE);

  pinMode (D7, INPUT_PULLUP);
}

void loop() {
  server.handleClient();
  btn = digitalRead (D7);
  dist = 0.4 * (float)currentValue;
  if ((millis() - oldTime) > 100) {
    oldTime = millis();
    display.clearDisplay();
    display.setCursor(5, 10);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.println(((String)dist));
    display.setCursor(90, 10);
    display.println(F("CM"));
    if (dist >= 50)dist = 0;
    display.display();
  }
  if (dist != lastDist) {
    Serial.println(dist);
    lastDist = dist;
  }
  if (btn != lastBtn) {
    Serial.println(btn);
    lastBtn = btn;
  }
  if(btn==0){
    currentValue=0;
  }
}

void handle_getDist() {
  Serial.println("Get Distance");
  server.send(200, "text/html", (String)dist);
}

void handle_reset() {
  currentValue = 0;
  Serial.println("Reset Distance");
  server.send(200, "text/plain", "0.00");
}

ICACHE_RAM_ATTR void Encoder() {        // pin change interrupts service routine. interrupts
  //     automatically disabled during execution
  INDEX     = INDEX << 2;               // Shift previous state left 2 bits (0 in)
  if (digitalRead (aPIN)) bitSet(INDEX, 0); // If aPIN is high, set INDEX bit 0
  if (digitalRead (bPIN)) bitSet(INDEX, 1); // If bPIN is high, set INDEX bit 1
  CURRENT = INDEX & 3;                  // CURRENT is the two low-order bits of INDEX
  INDEX &= 15;                          // Mask out all but prev and current
  // INDEX is now a four-bit index into the 16-byte ENCTABLE state table
  TOTAL += ENCTABLE[INDEX];             //Accumulate transitions
  if ((CURRENT == 3) || ((CURRENT == 0) && encoderType)) { //A valid tick can occur only at a detent
    if (TOTAL == (ZEERO + THRESH)) {
      currentValue--;
    }
    else if (TOTAL == (ZEERO - THRESH)) {
      currentValue++;
    }
    TOTAL = ZEERO;                      //Always reset TOTAL to 0x80 at detent
  }
}
