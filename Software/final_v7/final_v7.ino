#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const uint32_t GPSBaud = 9600;


TinyGPSPlus gps;

SoftwareSerial neo6m(4, 5); //Tx, Rx
SoftwareSerial sim800l(7, 6); // Tx, Rx for SIM800L module

#define BUTTON_PIN 8 // Pin where the button is connected
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long lastBlinkTime = 0;
const long interval1 = 500; // Interval for LED1 blink (in milliseconds)
const long interval2 = 700; // Interval for LED2 blink (in milliseconds)
const int ledPin1 = 10; // pin for Led 1
const int ledPin2 = 11;// pin for Led 2
const int buzzerPin = 9; // Pin for buzzer 
const int trigPin1 = 13; // Ultrasonic Sensors Trig and Echo pins
const int echoPin1 = 12; //**
const int trigPin2 = 3;//**
const int echoPin2 = 2;//**
boolean buttonPressed = false; //Button State initially not pressed
int emergency_state=1; // Initializing Emergency State as 1


//---------------------------------------------------------------------------------------------
boolean getResponse(String expected_answer, unsigned int timeout = 1000) {
  boolean flag = false;
  String response = "";
  unsigned long previous;
  //*************************************************************
  for (previous = millis(); (millis() - previous) < timeout;) {
    while (sim800l.available()) {
      response = sim800l.readString();
      if (response.indexOf(expected_answer) > 0) {
        flag = true;
        goto OUTSIDE;
      }
    }
  }
  //*************************************************************
  OUTSIDE:
  if (response != "") { Serial.println(response); }
  return flag;
}
//---------------------------------------------------------------------------------------------
boolean sendATComm(String command, String expected_answer, int timeout, int total_tries) {
  TryAgain:

  for (int i = 1; i <= total_tries; i++) {
    sim800l.println(command);
    if (getResponse(expected_answer, timeout) == true) {
      break;
    }
    else {
      Serial.print(".");
    }
  }

}

void sendSMS(String number, String msg) {
  Serial.println("Calling Sim800l");
  sendATComm("AT+CMGS=\"" + String(number) + "\"", ">", 1000, 5); // This will try sending the message for 5 times
  sim800l.print(msg);
  sim800l.write(26); // ASCII code for Ctrl+Z to send the message
  delay(1000);
  buttonPressed = true; // Mark the button as pressed to avoid sending multiple SMS
  Serial.println("Button Pressed"); 
}

void buzzer(){
  tone(buzzerPin, 1000); // 1000 Hz tone
  delay(100);
  noTone(buzzerPin);// Turn off the buzzer for 100 milliseconds
  delay(100);
  // Repeat the process for a different tone
  tone(buzzerPin, 2000); // 2000 Hz tone
  delay(100);
  noTone(buzzerPin);
  delay(100);
}

void ultrasonic(int trigPin, int echoPin, int buzzerPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  int distance = duration * 0.034 / 2; // Calculating distance using formula in the datasheet
  Serial.print("Distance from the object = ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 25) { // If distance is less than 25, ring buzzer
    buzzer();
  }
}


void gps_location(){
  neo6m.begin(GPSBaud); // Start communication with Arduino 
  neo6m.listen(); // Call the Module
  Serial.println("Calling GPS");
  unsigned long startMillis = millis();
  while (millis() - startMillis < 1000) { // Wait for 1 second
    while (neo6m.available()) {
      gps.encode(neo6m.read());
      if (gps.location.isUpdated()) { // Formatting the NMEA sentences
        Serial.print("Latitude= "); 
        Serial.print(gps.location.lat(), 6);
        Serial.print(" Longitude= "); 
        Serial.println(gps.location.lng(), 6);
        return; 
      }
    }
  neo6m.end(); // End communication with the Module
  }
  Serial.println("No GPS data available, help required");
}

void blinkLED(){
  unsigned long currentMillis = millis(); // Get the current time

  // Blink LED1
  if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis; // Save the last time LED1 was blinked
    digitalWrite(ledPin1, !digitalRead(ledPin1)); // Toggle LED1
  }

  // Blink LED2
  if (currentMillis - previousMillis2 >= interval2) {
    previousMillis2 = currentMillis; // Save the last time LED2 was blinked
    digitalWrite(ledPin2, !digitalRead(ledPin2)); // Toggle LED2
  }
}


void setup(){
  Serial.begin(9600); // Initialise the communication with Arduino
  // Declare the modes for all different Pins
  pinMode(ledPin1, OUTPUT); 
  pinMode(ledPin2, OUTPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT); 
}

void loop(){

  ultrasonic(trigPin1, echoPin1, buzzerPin); // Calculates distance from the 1st Sensor
  ultrasonic(trigPin2, echoPin2, buzzerPin); // Calculates distance from the 2nd Sensor
  delay(100);

  if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) { 
    emergency_state=0; // in emergency state, state is changed from 1 to 0
    gps_location(); //call gps function for location
    sim800l.begin(9600); // start comm with sim800l
    sim800l.listen(); // call the sim800l module
    String message; // declare local variable for mesasge
    message = "HELP ME!!! https://www.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    Serial.println(message); 
    sendSMS("447888196873", message); // send SMS to this number
    lastBlinkTime = millis(); // Capture the time for counting later for led's

  }

  if(emergency_state!=1){
    
    if (millis() - lastBlinkTime < 600000) {  // Check if 10 minutes have passed since the LEDs started blinking
    
      blinkLED(); // Blink LED function to blink LED's
      ultrasonic(trigPin1, echoPin1, buzzerPin); // Keep scanning for obstacles
      ultrasonic(trigPin2, echoPin2, buzzerPin);
      
    }
    else{
      digitalWrite(ledPin1, LOW); // Turn the led's off
      digitalWrite(ledPin2, LOW);
      emergency_state=1; // Back to normal state
    }
  }
}



