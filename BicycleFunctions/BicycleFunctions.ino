/*********************************************************
 * Armond Luthens
 * Charles Rathe
 * Embedded Systems Final Project
 * University of Iowa
 * Project: Bicycle Light Up Numbers Tracker
 *********************************************************/


/*********************************************************
 * Library Imports
 *********************************************************/
#include <LiquidCrystal.h>


/*********************************************************
 * Global Variables
 *********************************************************/
float currentSpeed = 0.0;
float averageSpeed = 0.0;
float tripDistance = 0.0;
float tripTime = 0.0;
float currentSlope = 0.0;
unsigned int cadence = 0;
unsigned int pulseCount = 0;

unsigned timeElapsed = 0;
unsigned previousTime = 0;

//const float wheelDiameter = 584.0; //millimeters
const float wheelCircumference = 6.15; // measured in feet --> [(23.5/12)*pi]
/*********************************************************
 * Reed Switch
 *********************************************************/
const int reedSwitch = 10;

/*********************************************************
 * LCD
 *********************************************************/
LiquidCrystal lcd(13, 11, A0, A1, A2, A3);

/*********************************************************
 * State Info:
 * State 0: Current Speed
 * State 1: Average Speed
 * State 2: Trip Distance
 * State 3: Current Cadence
 * State 4: Current Slope
 *********************************************************/
short state=0;

/*********************************************************
 * State Change Push Button
 *********************************************************/

const int stateButton = 2; //state push button on pin 2
const int resetButton = 3; //reset push button on pin 3

void setup() {
  // put your setup code here, to run once:
  
  pinMode(reedSwitch, INPUT);                    //Configure reed pin as an input

  //External Interrupt for State Change
  pinMode(stateButton, INPUT);                //Configure state button (on pin 2) as an input
  attachInterrupt(0, changeState, CHANGE);    //Attach interrupt to pin on change

  
  //External Interrupt for Reset Button
  pinMode(resetButton, INPUT);                 //Configure reset button (on pin 3) as an input
  attachInterrupt(0, resetAllValues, CHANGE);  //Attach interrupt to pin on change

  // Set up LCD
  //  lcd.begin(16,2);            // Set LCD for 16 columns, 2 lines
  //  lcd.clear();                // Clear LCD and print intro
}

void loop() {
  checkReedSwitch();
}

/*********************************************************
 * ISR Functions
 *********************************************************/
void changeState(){
    if(state == 4){
        state = 0;
    }
    else{
        state++;
    }
}

void resetAllValues(){
    currentSpeed = 0.0;
    averageSpeed = 0.0;
    tripDistance = 0.0;
    tripTime = 0.0;
    currentSlope = 0.0;
}
/*********************************************************
 * Reed Switch
 *********************************************************/
void checkReedSwitch(){
    int reedState = digitalRead(reedSwitch);
    if(reedState == HIGH){
        pulseCount++;
        timeElapsed = millis() - previousTime;
        previousTime = millis();
    }
    
    getCurrentSpeed();
    getAverageSpeed();
    getTripDistance();
    getCurrentCadence();
    getCurrentSlope(); 
}

/******************************************
 * Write To LCD Screen
 ******************************************/
void writeToLCD(){
    
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Trip Time: ");
   
    if(state == 0){
        lcd.setCursor(1, 1);
        lcd.print("Speed: ");
        lcd.setCursor(8, 1);
        lcd.print(currentSpeed, 10);
        lcd.setCursor(10, 1);
        lcd.print(" mph");
    }
    else if(state == 1){
        lcd.setCursor(1, 1);
        lcd.print("Avg Speed: ");
        lcd.setCursor(12, 1);
        lcd.print(averageSpeed, 10);
        lcd.setCursor(14, 1);
        lcd.print("mph");
    }
    else if(state == 2){     
        lcd.setCursor(1, 1);
        lcd.print("Distance: ");
        lcd.setCursor(11, 1);
        lcd.print(tripDistance, 10);
        lcd.print(" mi");
    }
    else if(state == 3){
        lcd.setCursor(1, 1);
        lcd.print("Cadence: ");
        lcd.setCursor(10, 1);
        lcd.print(cadence, 10);
        lcd.setCursor(13, 1);
        lcd.print(" RPM");
    }
    else{
        lcd.setCursor(1, 1);
        lcd.print("Slope: ");
        lcd.print(currentSlope);
        lcd.print(" deg");
    }
}

/******************************************
 * Total Trip Time
 ******************************************/
void getTripTime(){
  
}

/******************************************
 * Bike Functions
 ******************************************/

/******************************************
 * Computes Current Speed Of Bike
 * in miles per hour
 * Status: Complete
 ******************************************/
void getCurrentSpeed(){
    
    currentSpeed = wheelCircumference/timeElapsed;
    currentSpeed = currentSpeed * 681.8181;
    
    if(state == 0){
          writeToLCD();
    }
}


/******************************************
 * Computes Average Speed of the Bike
 * in miles per hour
 * Status: COMPLETE
 ******************************************/
void getAverageSpeed(){
    averageSpeed = ((averageSpeed * pulseCount) + currentSpeed)/(pulseCount + 1);
    if(state == 1){
          writeToLCD();
    }
}


/******************************************
 * Computes Total Trip Distance
 * in miles
 * Status: COMPLETE
 ******************************************/
void getTripDistance(){
    tripDistance = (wheelCircumference * pulseCount)/5280;
    
    if(state == 2){
        writeToLCD();
    }
}

/******************************************
 * Computes Current Cadence of Bike
 * in revolutions/minute
 * Status: COMPLETE
 ******************************************/
void getCurrentCadence(){
    cadence = 60*(1000/timeElapsed);
     
    if(state == 3){
          writeToLCD();
    }
}

/******************************************
 * Computes Current Slope Of Bike
 * in degrees
 * Status: NOT Complete
 ******************************************/
void getCurrentSlope(){
    
    if(state == 4){
          writeToLCD();
    }
}















