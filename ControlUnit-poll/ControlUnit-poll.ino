/***********************************************************************
*  Program: Control Unit Firmware for the GunSafe EasyUse Locking System 
*
* Authors: Jake Lantz, Charles Rathe,    (software specialty)
*        Phil Partipilo, Nick Bertram  (hardware specialty)
*
* Description: Firmware which recieves input from sensor via  rf transmitter
*        and controls most of the basic functions of 
*        the GunSafe EasyUse Alarm System, including:
*         Keypad -> Using Keypad library
*         LCD -> Using LiquidCrystal library
*         RF Receier -> Using VirtualWire library
*
************************************************************************/

// Referenced Libraries
  #include "Keypad.h"
  #include "LiquidCrystal.h" 
  #include "EEPROM.h"
  #include "VirtualWire.h"
  #include "pitches.h"

// Global Variables
  const int eeAddr = 1;                   // Byte of EEPROM where start of PIN is
  const int digitAddr = 0;                // Byte of EEPROM where number of digits in PIN is stored: if 0, pick PIN
  const int rxPin = 9;                    // Defines which PIN recieves data from transmitter
  const int alarmPin = 10;                // Defines PIN which alarm is sent to
  
  const float defaultTH = 70;             // Default pressure threshhold for alarm (V)
  
  int addr;                               // Holds current address
  int STATE = 0;                          // Defines state of system (Disarmed -> 1)
  int pin_digits;                         // Number of digits in the pin
  int option = 0;                         // Holds current menu option
  int count = 0;                          // Generic counting variable
  int digit = 0;                          // Temporarily holds digits
  int currentTH = 70;                     // Current threshhold set by user (V)
  int reading;                            // Holds reading from rf
  
  bool selecting;                         // Determines whether the user is selecting an option
  bool entering;                          // Determines whether the user is entering a PIN
  bool isValid;                           // Return value after validating PIN
  
  char key;                               // Holds characters read from KeyPad
  char newPIN[9];                         // Holds user-entered PIN

// Set up Keypad and LCD
  const byte ROWS = 4;
  const byte COLS = 3;

  char keys[ROWS][COLS] =  
  {{'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}};

  byte rowPins[ROWS] = {5, 4, 3, 2};
  byte colPins[COLS] = {8, 7, 6};

  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
  LiquidCrystal lcd(13, 11, A0, A1, A2, A3);

////////////////////////////////////////////////////////////////////
//                          Setup                                 //
////////////////////////////////////////////////////////////////////

void setup(){
  
// Set up Serial (debugging)
  Serial.begin(9600);
  
// Set up pin I/O
  pinMode(rxPin, INPUT);  
  pinMode(alarmPin, OUTPUT);
  digitalWrite(10, LOW);

// Set up RF Transmitter and Timer 0
  setup_transmitter(); 
  digitalWrite(10, LOW);             

// Set up LCD
  lcd.begin(16,2);            // Set LCD for 16 columns, 2 lines
  lcd.clear();                // Clear LCD and print intro

// Short delay, then go to setup state
  delay(50);
  state_zero();  

}

/////////////////////////////////////////////////////////////////////
//                            Main                                 //
/////////////////////////////////////////////////////////////////////

void loop(){

// If not alarm state, go to menu
  if(STATE != 3)
    menu_state();
    
// Else need to turn off alarm
  else{
    if(enterPIN()){
      STATE = 1;
      digitalWrite(alarmPin, LOW);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
//                          STATE Functions                            //
/////////////////////////////////////////////////////////////////////////

/************
 * STATE 0  |
 ***********/
void state_zero(){
  
// Display Welcome message
    print_welcome();
    
// Read in new PIN # from user
    enterNewPIN();

// Set state to 1
    STATE = 1;

// Print instructions
    print_instructions();
}

/***********
 * STATE 1 |
 **********/
void menu_state(){
  
  selecting = true;
  option = 0;

  // Print out menu
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("MENU: ");
  
  // Wait for selection
  while(selecting){
    entering = true;
    
  // This option arms or disarms the safe depending on STATE
    if(option == 0){
      lcd.clear();
      lcd.print("MENU:");
      lcd.setCursor(1,1);
      
      if(STATE == 1)
        lcd.print("<-    ARM    ->");
        
      else if(STATE == 2)
        lcd.print("<-   DISARM  ->");
    }

  // This option allows the user to change the PIN
    else if(option == 1){
      lcd.setCursor(1,1);
      lcd.print("<- CHANGEPIN ->");
    }

  // This option allows the user to sound the alarm and check it
    else if(option == 2){
      lcd.setCursor(1,1);
      lcd.print("<- CHECKALARM->");
    }

  // This option allows the user to change the alarm to one of several presets
    else{
      lcd.setCursor(1,1);
      lcd.print("<- CALIBRATE ->");
    }

    // Get user selection
     while(entering){
        get_selection();
     }
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//                           Transmission Functions                                //
/////////////////////////////////////////////////////////////////////////////////////
bool check_pressure(){
  bool pressure = true;
  
  if(STATE == 2){

    uint8_t buf[4];
    uint8_t buflen = 4;
    String msg = "";
    
    vw_rx_start();
    
    if(vw_get_message(buf, &buflen)){
        
      // Print out recieved data (debugging)
        Serial.print("Got: ");
    
        for(int i=0; i<buflen; i++){
          msg += (char) buf[i];
          Serial.print(msg[i]);
          Serial.print(' ');
        }

        Serial.print("    String: ");
        Serial.print(msg);

         if(msg.toFloat() <= 1.0)
            pressure = false;
    
        Serial.println();
      }
  }
  return pressure;
}

void setup_transmitter(){

// Configure pins/settings
  vw_set_rx_pin(rxPin);
  vw_set_ptt_inverted(true);
  vw_setup(2000);
}

/////////////////////////////////////////////////////////////////////////////////////
//                                Display Functions                                //
/////////////////////////////////////////////////////////////////////////////////////

void print_welcome(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Welcome!");
  lcd.setCursor(1,1);
  delay(500);
  lcd.print(".  ");
  delay(500);
  lcd.print(".  ");
  delay(500);
  lcd.print(". ");
  delay(500);
}

void print_instructions(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Please select");
  lcd.setCursor(1,1);
  lcd.print("an option");
  delay(2000);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("4 & 6 to change");
  lcd.setCursor(1,1);
  lcd.print("'#' to select");
}

void print_disable_alarm(){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print(" ***TESTING*** ");
  lcd.setCursor(1,1);
  lcd.print("  '#' to stop  "); 
  delay(2000);
}

void print_arming(){
  lcd.clear();
  lcd.print("Arming");
  delay(500);
  lcd.setCursor(1,1);
  lcd.print(". ");
  delay(500);
  lcd.print(". ");
  delay(500);
  lcd.print(". ");
  delay(500);
}

void print_disarming(){
  lcd.clear();
  lcd.print("Disarming");
  delay(500);
  lcd.setCursor(1,1);
  lcd.print(". ");
  delay(500);
  lcd.print(". ");
  delay(500);
  lcd.print(". ");
  delay(500);
}

void print_incorrect_PIN(){
  lcd.clear();
  lcd.print("PIN Incorrect!");
  delay(2000);
}
  
//////////////////////////////////////////////////////////////////////////////
//                            Alarm Functions                               //
//////////////////////////////////////////////////////////////////////////////

void test_alarm(){

    print_disable_alarm();
    delay(100);
    digitalWrite(10, HIGH);
    bool testing = true;

    while(testing){

      // Get input from keypad
      key = keypad.getKey();

      // If input is '#' -> Stop testing
      if(key != NO_KEY){
        if(key == '#')
          testing = false;  
      }
    }
    
    digitalWrite(10, LOW);
}

void calibrate_alarm(){

  
}

/////////////////////////////////////////////////////////////////////////////////
//                             PIN Functions                                   //
/////////////////////////////////////////////////////////////////////////////////

bool enterPIN()
{
  // Get number of digits in PIN
  pin_digits = EEPROM.read(digitAddr);

  // Scope Variables
  isValid = true;
  addr = eeAddr;
  entering = true;
  count = 0;

  // Set menu
  lcd.clear();
  lcd.print("Enter PIN");
  lcd.setCursor(1,1);

  // Get PIN
  while(1){
    
      // Get input from keypad
      key = keypad.getKey();

      // Check if a button has been pressed
      if(key != NO_KEY){
        
        // Print the key to the screen
        lcd.print(key);

        if(key == '*'){
          // Reset variables and LCD
          lcd.clear();
          lcd.print("Enter PIN:");
          lcd.setCursor(1,1);
          count = 0;
          
          // Clear array holding pin
          for(int i = 0; i < pin_digits; i++)
            newPIN[i] = 0;
      }

      // User wants to exit 'enter pin' mode
      else if(key == '#')
        return false;

      // If number entered, store it in array
      else{

        digit = key - 48;
        newPIN[count++] = digit;

        // Check if the correct number of digits has been entered
        if(count == pin_digits)
        { 
          for(int i = 0; i < pin_digits; i ++)
          {
            if (!(newPIN[i] == EEPROM.read(addr++)))
              isValid = false;
          }
          return isValid;
        }
      }
    }
  }
}

void enterNewPIN()
{
  // Scope Variables
  addr = eeAddr;
  entering = true;
  count = 0;
  
  lcd.clear();
  lcd.print("Enter # of digits:");
  lcd.setCursor(1,1);

  while(entering)
  {
    // Get input from keypad
    key = keypad.getKey();

    // Check if a button has been pressed
    if(key != NO_KEY)
    {
      // Print the key to the screen
      lcd.print(key);

      if(!(key == '*' || key == '#'))
      {
        // Convert ASCII character to digit and store in EEPROM
        digit = key-48;

        if(digit >= 1){
          EEPROM.write(digitAddr, digit);

        // Store new digits locally and exit loop
        pin_digits = digit;
        entering = false;
        }

        else{
          lcd.clear();
          lcd.setCursor(1,0);
          lcd.print("Must be > 1!");
          delay(2000);
        }
      }
    }      
  }

  // Reset variables and LCD
  entering = true;
  lcd.clear();
  lcd.print("Enter new PIN:");
  lcd.setCursor(1,1);

  // While user is still entering
  while(entering)
  {
    // Get input from keypad
    key = keypad.getKey();

    // Check if a button has been pressed
    if(key != NO_KEY)
    {
      // Print the key to the screen
      lcd.print(key);

      // Check if user has pressed '*' (clear)
      if(key == '*')
      {
        // Clear bottom line of LCD, reset count
        lcd.clear();
        lcd.print("Enter new PIN:");
        lcd.setCursor(1,1);
        count = 0;

        // Clear array holding pin
        for(int i = 0; i < pin_digits; i++)
        {newPIN[i] = 0;}
      }

      // Check if number entered
      else{

        digit = key - 48;
        // Store in array (increase count)
        newPIN[count++] = digit;

        // Check if the correct number of digits has been entered
        if(count == pin_digits)
        { 
          for(int i = 0; i < pin_digits; i ++)
          {
            EEPROM.write(addr++, newPIN[i]);
          }
          entering = false;
        }
        selecting = false;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//                              Keypad Functions                           //
/////////////////////////////////////////////////////////////////////////////

void get_selection(){

  if(!check_pressure()){
    digitalWrite(alarmPin, HIGH);
    STATE = 3;
    selecting = false; 
    entering = false;
  }

 key = keypad.getKey();
  
  // Check if key is pressed
  if (key != NO_KEY)
  {
    // If key is 6, increment menu
    if(key == '6'){
      if(option != 3)
          option += 1;
      else
          option = 0;
    }
    
    // If key is 4, decrement menu
    else if(key == '4'){
      if(option != 0)
          option -= 1;
      else
          option = 3;
    }

    // If '#' select option
    else if(key == '#')
    {
          if(option == 0){
            if(enterPIN()){

              if(STATE == 1){
                print_arming();
                STATE = 2 ;
              }

              else if(STATE == 2){
                print_disarming();
                STATE = 1;
                digitalWrite(10, LOW);
              }
            }

            else
              print_incorrect_PIN();
          }
            
          else if(option == 1){
            if(enterPIN)
              enterNewPIN();
          }
           
          else if(option == 2){
            test_alarm();}

          else
            calibrate_alarm();

            selecting = false;
    }
    entering = false; 
  }
}
