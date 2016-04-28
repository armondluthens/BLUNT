/************************************************
 *     LCD Test - Embedded Project
 *     Armond Luthes and Charles Rathe
 *     Smoke Blunts Everyday
 ***************************************************/

// Included Libraries
  #include <LiquidCrystal.h>
  #include <EEPROM.h>

// Global Variables

    //LiquidCrystal lcd(6, 5, 1, 2, 3, 4);
LiquidCrystal lcd(9, 8, 4, 5, 6, 7);
void setup() {

// Serial Setup (Debugging only)
  Serial.begin(9600);

  
// Set up LCD Display
  lcd.begin(16,2);
  lcd.clear();

  lcd.print("Hello World!");

}

void loop() {


}
