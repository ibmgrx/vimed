void main_menu(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(35, 0);
    display.println("DEVICE INFO");
    display.drawLine(0, 10, 120, 10, WHITE);
    display.setCursor(0, 20);
    display.println("IP:");
    display.setCursor(20, 20);
    display.println(IP);
    display.display();
}

void homescreen(){
  
}
