
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
#include <TimerOne.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MMA8451.h>
#include <math.h>
#include <Wire.h>

/*********************************************************
 * Global Variables
 *********************************************************/
float currentSpeed = 0.0;
float averageSpeed = 0.0;
float initialAverageSpeed = 0.0;
float totalSpeed = 0.0;

unsigned int totalSeconds = 0;

float tripDistance = 0.0;
unsigned int tripSeconds = 0;
unsigned int tripMinutes = 0;
unsigned int tripHours = 0;

int currentSlope = 0;
unsigned int cadence = 0;
unsigned int cadenceCount = 0;
unsigned int pulseCount = 0;

unsigned int pulseCount2 = 0;

unsigned timeElapsed = 0;
unsigned previousTime = 0;
unsigned timeCheck = 0;

unsigned int timerCount = 0;

//const float wheelDiameter = 584.0; //millimeters
const float wheelCircumference = 6.15; // measured in feet --> [(23.5/12)*pi]
/*********************************************************
 * Reed Switch
 *********************************************************/
const int reedSwitch = 2;

/*********************************************************
 * Accelerometer Global Variables
 *********************************************************/
Adafruit_MMA8451 mma = Adafruit_MMA8451();

/*********************************************************
 * LCD
 *  LCD RS pin to digital pin 9
    LCD Enable pin to digital pin 8
    LCD D4 pin to digital pin 4
    LCD D5 pin to digital pin 5
    LCD D6 pin to digital pin 6
    LCD D7 pin to digital pin 7
 *********************************************************/
LiquidCrystal lcd(9, 8, 4, 5, 6, 7);


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
const int stateButton = 3; //state push button on pin 2

/*********************************************************
 * PROGRAM SETUP
 *********************************************************/
void setup() { 
    Serial.begin(9600);

    //set up timer
    Timer1.initialize(1000000);
    Timer1.attachInterrupt(timer_ISR);

    
    //set up interrupt for magnetic reed switch
    pinMode(reedSwitch, INPUT);                    //Configure reed pin as an input
    attachInterrupt(digitalPinToInterrupt(reedSwitch), reedSwitchISR, RISING);    //Attach interrupt to pin on change
    
    //External Interrupt for State Change
    pinMode(stateButton, INPUT);                //Configure state button (on pin 2) as an input
    attachInterrupt(1, changeState, RISING);    //Attach interrupt to pin on change


    //Pin change interrupt for reset all function
    cli();              // disable global interrupts
    PCMSK0 |= 1<<PCINT4;
    PCICR |= 1<<PCIE0;
    sei();

    //set up accelerometer
    mma.begin();
    mma.setRange(MMA8451_RANGE_2_G);
    
    //Set up LCD
    lcd.begin(16,2);            // Set LCD for 16 columns, 2 lines
    lcd.clear();                // Clear LCD and print intro
    lcd.setCursor(0, 0);
    lcd.print("Trip Time: ");
}

void loop() {
  getCurrentSlope();    //get accelerometer reading
  delay(1000);          //delay 1 second in between readings
}

/*********************************************************
 * ISR Functions
 *********************************************************/
/********************************************
 * Magnetic Reed Switch External Interrupt
 * Pulses everytime wheel comes around
 ********************************************/
void reedSwitchISR(){
    pulseCount++;
    pulseCount2++;
    timeElapsed = millis() - previousTime;
    previousTime = millis();

    getCurrentSpeed();
    getTripDistance();
}


/********************************************
 * Change State Button External Interrupt
 * Change current state (bicycle function)
 ********************************************/
void changeState(){
    if(state == 4){
        state = 0;
    }
    else{
        state++;
    }
    writeToLCD();   //update state change on LCD
}

/*******************************
 * Reset Button PC Interrupt
 * Reset all values on bike
 * computer
 *******************************/
ISR(PCINT0_vect) {
    currentSpeed = 0.0;
    averageSpeed = 0.0;
    tripDistance = 0.0;
    tripSeconds = 0;
    tripMinutes = 0;
    tripHours = 0;
    currentSlope = 0.0;
    totalSeconds = 0;
    pulseCount = 0;
    pulseCount2 = 0;

    writeToLCD();
}


/*******************************
 * Timer Overflow Interrupt
 *******************************/
void timer_ISR(){
    
    totalSeconds++;                 //keeps track of total seconds used for average speed
    getAverageSpeed();              //update average speed
    if(timeCheck == previousTime){  //if speed is not changing
        currentSpeed = 0;
    }
    timeCheck = previousTime;

    if(cadenceCount == 3){
        cadence = 20*pulseCount2;   //update cadence every 3 seconds
        cadenceCount = 0;
        pulseCount2 = 0;
    }
    else{
        cadenceCount++;
    }
    
    writeToLCD();   //write all updates to LCD

    //update timer
    if(tripSeconds != 59){
        tripSeconds++;
    }
    else{
        tripSeconds = 0;
        
        if(tripMinutes == 59){
           tripMinutes = 0;
           tripHours++;
        }
        else{
            tripMinutes++;
        }
    }      
}

/******************************************
 * Write To LCD Screen
 ******************************************/
void writeToLCD(){

    //Print the trip time (always on first line of LCD)
    lcd.clear();
    lcd.setCursor(0, 0);
    char times[9];
    sprintf(times, "%02d:%02d:%02d", tripHours, tripMinutes, tripSeconds);
    lcd.print(times);

    //Print the selected function based on state
    if(state == 0){
        lcd.setCursor(0, 1);
        lcd.print("Speed: ");
        lcd.print(currentSpeed);
        lcd.print(" mph");
    }
    else if(state == 1){
        lcd.setCursor(0, 1);
        lcd.print("Avg S: ");
        lcd.print(averageSpeed);
        lcd.print(" mph");
    }
    else if(state == 2){     
        lcd.setCursor(0, 1);
        lcd.print("Dist: ");
        lcd.print(tripDistance);
        lcd.print(" mi");
    }
    else if(state == 3){
        lcd.setCursor(0, 1);
        lcd.print("Cadence: ");
        lcd.print(cadence);
        lcd.print(" RPM");
    }
    else{
        lcd.setCursor(0, 1);
        lcd.print("Slope: ");
        lcd.print(currentSlope);
        lcd.print("%");
    }
}


/************************************************************
 * Bike Functions
 ************************************************************/

/******************************************
 * Computes Current Speed Of Bike
 * in miles per hour
 * Status: Complete
 ******************************************/
void getCurrentSpeed(){
    currentSpeed = wheelCircumference/timeElapsed;
    currentSpeed = currentSpeed * 681.8181;
}


/******************************************
 * Computes Average Speed of the Bike
 * in miles per hour
 * Status: COMPLETE
 ******************************************/
void getAverageSpeed(){
  
  double temp = ((totalSeconds*1000)/3600);

  if(totalSeconds > 0){
    averageSpeed = tripDistance/temp;
    averageSpeed = averageSpeed*1000;
  }
  else{
    averageSpeed = 0;
  }
    
}

/******************************************
 * Computes Total Trip Distance
 * in miles
 * Status: COMPLETE
 ******************************************/
void getTripDistance(){
    tripDistance = (wheelCircumference * pulseCount)/5280;
}

/******************************************
 * Computes Current Cadence of Bike
 * in revolutions/minute
 * Status: COMPLETE
 ******************************************/
void getCurrentCadence(){
    cadence = 60*(1000/timeElapsed);
}

/******************************************
 * Computes Current Slope Of Bike
 * in degrees
 * Status: NOT Complete
 ******************************************/
void getCurrentSlope(){
    float tempSlope;
    mma.read();
    sensors_event_t event; 
    mma.getEvent(&event);
    tempSlope = acos(event.acceleration.z/9.8);
    currentSlope = tan(tempSlope)*100;
}





