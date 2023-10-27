//************************************
void homescreen() {
  display.clearDisplay();
  display.setCursor(37, 7);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println(F("VIMED"));
  display.setCursor(115, 25);
  display.setTextSize(1);
  display.println(F("OK"));
  display.display();
}

void testscrolltext(String text) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 7);
  display.println(text);
  display.display();
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  display.startscrollleft(0x00, 0x0F);
  delay(800);
  display.stopscroll();
}

int count = 0;
uint8_t main_menu() {
  if (btn1 == LOW) {
    count++;
    if (count > 3) count = 0;
    delay(250);
  }
  display.clearDisplay();
  display.setTextSize(1);
  for (int i = 0; i < numMenuItems; i++) {
    if (i == count) {
      display.setCursor(5, i * 8);
      display.print("->");
    } else {
      display.setCursor(5, i * 8);
      display.print("");
    }
    display.setTextSize(1);
    display.setCursor(25, i * 8);
    display.print(menuItems[i]);
  }
  display.display();
  return count;
}

void screen_value(float value) {
  display.clearDisplay();
  display.setCursor(5, 5);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println(((String)value));
  display.setCursor(90, 5);
  display.println(F("CM"));
  display.display();
}

void screen_saved(){
  display.clearDisplay();
  display.setCursor(10, 5);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.println(F("SAVED"));
  display.display();
}

void screen_data(float a, float b, float c){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(("A: "+(String)a+" cm"));
  display.setCursor(0, 10);
  display.println(("B: "+(String)b+" cm"));
  display.setCursor(0, 20);
  display.println(("C: "+(String)c+" cm"));
  display.display();
}