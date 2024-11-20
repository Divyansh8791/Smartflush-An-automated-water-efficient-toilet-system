#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MPR121.h>


#define MINUTE 60000 // for converting milliseconds to a minute
#define SECOND 1000 // for converting milliseconds to a second

Adafruit_MPR121 mpr121 = Adafruit_MPR121();   // Create an MPR121 object to communicate with the sensor 
int mpr_pin = 7;                              // MPR pin for MPR -121 touch sensor.

Servo myServo;                                // Create an MPR121 object to communicate with the Servo Motor  
int servo_pin = 9;
// Variable to store capacitance value
uint16_t capacitanceValues[1];  // Array of Electrods.

int ledPin = 13;                // pin for the LED to sense the motion.
int pirPin = 2;                // signal pin of the PIR sensor

int trig_pin = 11;            // Trig pin for HCSR-04 distance sensor.
int echo_pin =  10;           // Echo pin for HCSR-04 distance sensor.
 

int lastPirVal = LOW;          // the last value of the PIR sensor
int pirVal;                    // the current value of the PIR sensor

unsigned long myTime;          // number of milliseconds passed since the Arduino started running the code itself
char printBuffer[128];         

// Checking the water level in tank.
int water_tank_level(){
 long duration, distance;

  // Send a pulse to the Trigger pin to start the measurement
  digitalWrite(trig_pin, LOW);  // Ensure the trigger is low
  delayMicroseconds(2);  // Small delay to ensure LOW signal
  digitalWrite(trig_pin, HIGH);  // Send pulse
  delayMicroseconds(10);  // Pulse duration
  digitalWrite(trig_pin, LOW);  // End the pulse

  // Measure the time it takes for the echo to return
  duration = pulseIn(echo_pin, HIGH);  // Time in microseconds

  // Calculate the distance based on the time duration
  distance = (duration / 2) / 29.1;  // Convert time to distance (cm)

  // // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(500);  // Wait before next reading
  return distance;
}

//  Open the lid .
void lid_open(){ 
   myServo.write(180);
   Serial.println("Lid Opened Successfully.");
}

// Close the lod.
void lid_close(){ 
  myServo.write(0);
  Serial.println("Lid closed Successfully.");
}

//  flush the water according to mpr value.
void flush_water(int timing){
if(timing == 1){
  digitalWrite(mpr_pin, HIGH);
  delay(5000);
  digitalWrite(mpr_pin, LOW);
}
if(timing == 0){
  digitalWrite(mpr_pin, HIGH);
  delay(2000);
  digitalWrite(mpr_pin, LOW);
}
}

//  Performing flush operation.
void flush_operation(){

  capacitanceValues[0] = mpr121.filteredData(0);
  Serial.println("Capacitance values for each electrode:");
  Serial.println(capacitanceValues[0]);
  int val = capacitanceValues[0];
  if(val >= 25 && val <= 40){
    flush_water(1);
  }
  if(val >= 370 && val <= 385){
    flush_water(0);
  }
}
void setup() {
  pinMode(ledPin, OUTPUT);    // declare LED as output
  pinMode(mpr_pin, OUTPUT);   // declare mpr as output
  
  pinMode(pirPin, INPUT);     // declare PIR sensor as input for motion detector sensor.
  pinMode(trig_pin, OUTPUT);  // declare trig_pin as output.
  pinMode(echo_pin, INPUT);   // declare echo_pin as input.
  myServo.attach(servo_pin);
Serial.begin(9600);

// Check if mpr sensor working.
  if (!mpr121.begin()) {
    Serial.println("Couldn't find MPR121 sensor!");
    while (1);
  }
  Serial.println("MPR121 initialized successfully!");
  
}
 
void loop(){
  pirVal = digitalRead(pirPin);  // read current input value

  if (pirVal == HIGH) { // movement detected  
    digitalWrite(ledPin, HIGH);  // turn LED on
    delay(2000);
     if(water_tank_level() <= 20){
    lid_open();
     }
    if (lastPirVal == LOW) { // if there was NO movement before
      myTime = millis();
      sprintf(printBuffer, "%lu min %lu sec: Motion detected!", myTime/MINUTE, myTime%MINUTE/SECOND); 
      Serial.println(printBuffer);
      lastPirVal = HIGH;
    }
  } else { // no movement detected
    digitalWrite(ledPin, LOW); // turn LED off
       delay(5000);
    lid_close();
    if (lastPirVal == HIGH){ // if there was a movement before
      myTime = millis();
      sprintf(printBuffer, "%lu min %lu sec: Motion ended!", myTime/MINUTE, myTime%MINUTE/SECOND); 
      Serial.println(printBuffer);
      lastPirVal = LOW;
    }

    flush_operation();
  }
}