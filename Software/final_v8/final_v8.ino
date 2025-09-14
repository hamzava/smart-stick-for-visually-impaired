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
const long interval2 = 500; // Interval for LED2 blink (in milliseconds)
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
boolean checkResponse(String expectedResponse, unsigned long timeout = 1000) {
  boolean responseFlag = false;
  String serverResponse = "";
  unsigned long startTime;
  //*************************************************************
  for (startTime = millis(); (millis() - startTime) < timeout;) {
    while (sim800l.available()) {
      serverResponse = sim800l.readString();
      if (serverResponse.indexOf(expectedResponse) > 0) {
        responseFlag = true;
        goto END;
      }
    }
  }
  //*************************************************************
  END:
  if (serverResponse != "") { Serial.println(serverResponse); }
  return responseFlag;
}
//---------------------------------------------------------------------------------------------
boolean sendCommand(String cmd, String expectedResponse, int responseTimeout, int attempts) {
  Retry:

  for (int i = 1; i <= attempts; i++) {
    sim800l.println(cmd);
    if (checkResponse(expectedResponse, responseTimeout) == true) {
      break;
    }
    else {
      Serial.print(".");
    }
  }

}

void sendSMS(String number, String msg) {
  Serial.println("Calling Sim800l");
  sendCommand("AT+CMGS=\"" + String(number) + "\"", ">", 1000, 5); // This will try sending the message for 5 times
  sim800l.print(msg);
  sim800l.write(26); // ASCII code for Ctrl+Z to send the message
  buttonPressed = true; // Mark the button as pressed to avoid sending multiple SMS
  Serial.println("Button Pressed"); 
}

// Buzzer function for the right sensor
void buzzerRight(int distance) {
  // Define the sound pattern for the right sensor
  if (distance < 10) {
    tone(buzzerPin, 1000); // Play a tone for very close distance
    delay(150);
  } else if (distance < 20) {
    tone(buzzerPin, 500); // Play a different tone for moderately close distance
    delay(150);
  } else {
    noTone(buzzerPin); // Turn off the buzzer if the distance is beyond the defined ranges
  }
}

// Buzzer function for the left sensor
void buzzerLeft(int distance) {
  // Define the sound pattern for the left sensor
  if (distance < 10) {
    tone(buzzerPin, 1100); // Play a tone for very close distance
    delay(500);
  } else if (distance < 20) {
    tone(buzzerPin, 528); // Play a different tone for moderately close distance
    delay(500);
  } else {
    noTone(buzzerPin); // Turn off the buzzer if the distance is beyond the defined ranges
  }
}

void startupBuzzer(){
    // Unique sound sequence with the buzzer
  int notes[] = {262, 330, 392, 523}; // Define the frequencies for the notes
  int duration = 200; // Define the duration for each note (in milliseconds)

  for (int i = 0; i < 4; i++) { // Play each note in the sequence
    tone(buzzerPin, notes[i], duration); // Play the note
    delay(duration); // Pause between notes
    noTone(buzzerPin); // Turn off the buzzer
    delay(50); // Short pause between notes
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
  for (int i = 0; i < 3; i++) { // Blink 3 times
    blinkLED();
  }
  startupBuzzer();
}

void loop() {
  // Check emergency state
  if (emergency_state != 1) {
    buttonPressed = false;
    if (millis() - lastBlinkTime < 600000) {  // Check if 10 minutes have passed since the LEDs started blinking
      blinkLED(); // Blink LED function to blink LEDs
    } else {
      digitalWrite(ledPin1, LOW); // Turn off LEDs
      digitalWrite(ledPin2, LOW);
      emergency_state = 1; // Back to normal state
    }
  }

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
}


}



