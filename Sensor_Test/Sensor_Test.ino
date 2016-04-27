/************************************************
 *     Sensor Test - Embedded Project
 *     Armond Luthes and Charles Rathe
 *     Light em up up up, 
 *     Light em up up up,
 *     Light em up up up
 *     (I'm on fiyah)
 ***************************************************/

// Included Libraries
  #include <LiquidCrystal.h>
  #include <EEPROM.h>
  #include <Adafruit_MMA8451.h>
  #include <Adafruit_Sensor.h>
  #include <Wire.h>

// Global Variables

    Adafruit_MMA8451 mma = Adafruit_MMA8451();
    LiquidCrystal lcd(9, 8, 4, 5, 6, 7);

void setup() {

// Serial Setup (Debugging only)
  Serial.begin(9600);

// Check Sensor (Debug Version)
  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  
  Serial.println("MMA8451 found!");

  mma.setRange(MMA8451_RANGE_2_G);

  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
  
// Set up LCD Display
  lcd.begin(16,2);
  lcd.clear();

  lcd.print("Hello World!");

}

void loop() {

    // Read the 'raw' data in 14-bit counts
  mma.read();
  Serial.print("X:\t"); Serial.print(mma.x); 
  Serial.print("\tY:\t"); Serial.print(mma.y); 
  Serial.print("\tZ:\t"); Serial.print(mma.z); 
  Serial.println();

  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: \t"); Serial.print(event.acceleration.x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(event.acceleration.y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(event.acceleration.z); Serial.print("\t");
  Serial.println("m/s^2 ");
  
  /* Get the orientation of the sensor */
  uint8_t o = mma.getOrientation();
  
  switch (o) {
    case MMA8451_PL_PUF: 
      Serial.println("Portrait Up Front");
      break;
    case MMA8451_PL_PUB: 
      Serial.println("Portrait Up Back");
      break;    
    case MMA8451_PL_PDF: 
      Serial.println("Portrait Down Front");
      break;
    case MMA8451_PL_PDB: 
      Serial.println("Portrait Down Back");
      break;
    case MMA8451_PL_LRF: 
      Serial.println("Landscape Right Front");
      break;
    case MMA8451_PL_LRB: 
      Serial.println("Landscape Right Back");
      break;
    case MMA8451_PL_LLF: 
      Serial.println("Landscape Left Front");
      break;
    case MMA8451_PL_LLB: 
      Serial.println("Landscape Left Back");
      break;
    }
  Serial.println();
  delay(500);
 

}
