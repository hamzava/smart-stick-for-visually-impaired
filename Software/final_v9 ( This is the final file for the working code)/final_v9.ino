#include <TinyGPS++.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;

SoftwareSerial neo6m(4, 5); //Tx, Rx for neo6m
SoftwareSerial sim800l(7, 6); // Tx, Rx for SIM800L module

#define BUTTON_PIN 8 // Pin where the button is connected
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long lastBlinkTime = 0;
const long interval1 = 500; // Interval for LED1 blink (in milliseconds)
const long interval2 = 700; // Interval for LED2 blink (in milliseconds)
const int ledPin2 = 10; // pin for Led 1
const int ledPin1 = 11;// pin for Led 2
const int buzzerPin = 9; // Pin for buzzer 
const int trigPin1 = 13; // Ultrasonic Sensors Trig and Echo pins
const int echoPin1 = 12; //**
const int trigPin2 = 3;//**
const int echoPin2 = 2;//**
boolean buttonPressed = false; //Button State initially not pressed
int emergency_state=1; // Initializing Emergency State as 1
int ledState1 = LOW; // Initial state for LED1
int ledState2 = LOW; // Initial state for LED2

boolean checkResponse(String expectedResponse, unsigned long timeout = 1000) {
  boolean responseFlag = false; // flag for response
  String serverResponse = ""; // string variable to store response
  unsigned long startTime; 
  for (startTime = millis(); (millis() - startTime) < timeout;) { // millis function used to capture current time and take difference to get final time
    while (sim800l.available()) { //check's if sim800l is available or not
      serverResponse = sim800l.readString(); // stores the response from the server
      if (serverResponse.indexOf(expectedResponse) > 0) { // condition sets the flag to true if positive response
        responseFlag = true; 
        goto OUTSIDE;
      }
    }
  }
  OUTSIDE:
  if (serverResponse != "") { Serial.println(serverResponse); } // if the response is something else it will print that prompt and keep response flag as flase
  return responseFlag;
}
boolean sendCommand(String cmd, String expectedResponse, int responseTimeout, int attempts) {
  Retry:

  for (int i = 1; i <= attempts; i++) { // for loop to send command
    sim800l.println(cmd); 
    if (checkResponse(expectedResponse, responseTimeout) == true) { // if the condition is true the condition breaks and function is completed else it will print .... until attempts are completed
      break;
    }
    else {
      Serial.print(".");
    }
  }

}

void sendSMS(String number, String msg) { 
  Serial.println("Calling Sim800l");
  sendCommand("AT+CMGS=\"" + String(number) + "\"", ">", 1000, 5); // this will try sending the message for 5 times
  sim800l.print(msg); 
  sim800l.write(26); // ASCII code for Ctrl+Z to send the message
  buttonPressed = true; // mark the button as pressed to avoid sending multiple SMS
  Serial.println("Button Pressed"); 
}

// Buzzer function for the right sensor
void buzzerRight(int distance) {
  
  if (distance < 30) {
    tone(buzzerPin, 1000); // Play a tone 
    delay(150); // delay required for beeps
  } else if (distance < 45) {
    tone(buzzerPin, 500); // Play a different tone 
    delay(150);// delay required for beeps
  } else {
    noTone(buzzerPin); // Turn off the buzzer if the distance is more the defined ranges
  }
}

// Buzzer function for the left sensor
void buzzerLeft(int distance) {
  if (distance < 30) {
    tone(buzzerPin, 1100); // Play a tone 
    delay(500);
  } else if (distance < 45) {
    tone(buzzerPin, 528); // Play a different tone
    delay(500);
  } else {
    noTone(buzzerPin); // Turn off the buzzer if the distance is more the defined ranges
  }
}

void startupBuzzer(){
    // Unique sound sequence with the buzzer
  int notes[] = {262, 330, 392, 523}; // Frequencies for the notes in the array
  int duration = 200; // Duration of each note

  for (int i = 0; i < 4; i++) { // for loop to play each note in the array
    tone(buzzerPin, notes[i], duration); // play the note
    delay(duration); // pause between notes
    noTone(buzzerPin); // turn off the buzzer
    delay(50); // delay for pause
  }
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

  if (trigPin == trigPin1 && echoPin == echoPin1) {
    // If the right side sensor, produce a sound relative to the distance
    buzzerRight(distance);
  } else if (trigPin == trigPin2 && echoPin == echoPin2) {
    // If the left side sensor, produce a different sound pattern based on the distance
    buzzerLeft(distance);
  }
}


void gps_location(){
  neo6m.begin(9600);
  neo6m.listen();
  
  unsigned long startMillis = millis();
  while (millis() - startMillis < 1000) { // Wait for 1 second
    while (neo6m.available()) { // checks if the neo6m is available or not
      gps.encode(neo6m.read()); // reads data from neo6m if available
      if (gps.location.isUpdated()) { // if location is new following happnes
        Serial.print("Latitude= "); 
        Serial.print(gps.location.lat(), 6); // translate the received data into lattitude
        Serial.print(" Longitude= "); 
        Serial.println(gps.location.lng(), 6); // translate the received data into longitude
        return; // exit the function once location is updated
        neo6m.end(); // end communication with the Module
      }
    }
  }
  Serial.println("No GPS data received within 1 second");
}


void blinkLED(){

  unsigned long currentMillis = millis(); // Get the current time

  // Blink LED1
  if (currentMillis - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis; // Save the last time LED1 was blinked
    if (ledState1 == LOW) {
      digitalWrite(ledPin1, HIGH); // Turn LED1 on
      ledState1 = HIGH; // Update LED1 state
    } else {
      digitalWrite(ledPin1, LOW); // Turn LED1 off
      ledState1 = LOW; // Update LED1 state
    }
  }

  // Blink LED2
  if (currentMillis - previousMillis2 >= interval2) {
    previousMillis2 = currentMillis; // Save the last time LED2 was blinked
    if (ledState2 == LOW) {
      digitalWrite(ledPin2, HIGH); // Turn LED2 on
      ledState2 = HIGH; // Update LED2 state
    } else {
      digitalWrite(ledPin2, LOW); // Turn LED2 off
      ledState2 = LOW; // Update LED2 state
    }
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
  startupBuzzer(); // call this function to for sound indicating the system is up and running
}

void loop() {
  // Check for obstacles while checking emergency state
  ultrasonic(trigPin1, echoPin1, buzzerPin); // Calculates distance from the 1st Sensor
  ultrasonic(trigPin2, echoPin2, buzzerPin); // Calculates distance from the 2nd Sensor

  // Check button press for sending emergency message
  if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
    emergency_state = 0; // in emergency state, state is changed from 1 to 0
    gps_location(); // Call GPS function for location
    sim800l.begin(9600); // Start comm with SIM800L
    sim800l.listen(); // Call the SIM800L module
    String message = "HELP ME!!! https://www.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    Serial.println(message); 
    sendSMS("447888196873", message); // Send SMS to this number
    lastBlinkTime = millis(); // Capture the time for counting later for LEDs
    buttonPressed = true; // Mark the button as pressed
  }

    // Check emergency state
  if (emergency_state != 1) {
    sim800l.end();
    buttonPressed = false;
    if (millis() - lastBlinkTime < 300000) {  // Check if 5 minutes have passed since the LEDs started blinking
      blinkLED(); // Blink LED function to blink LEDs
    } else {
      digitalWrite(ledPin1, LOW); // Turn off LEDs
      digitalWrite(ledPin2, LOW);
      emergency_state = 1; // Back to normal state
    }

  }

}



