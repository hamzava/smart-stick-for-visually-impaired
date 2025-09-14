#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(4, 5); //Tx, Rx
SoftwareSerial sim800l(7, 6); // Tx, Rx for SIM800L module
#define BUTTON_PIN 8 // Pin where the button is connected
//#define RESET_PIN 13// 
const int ledPin1 = 10;
const int ledPin2 = 11;
const int buzzerPin = 9;
int state=1;
const int trigPin1 = 13;
const int echoPin1 = 12;
const int trigPin2 = 3;
const int echoPin2 = 2;
boolean buttonPressed = false;
unsigned long lastBlinkTime = 0;
int ledState = LOW;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
const long interval1 = 500; // Interval for LED1 blink (in milliseconds)
const long interval2 = 700; // Interval for LED2 blink (in milliseconds)
int ledState1 = LOW; // Initial state for LED1
int ledState2 = LOW; // Initial state for LED2


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
boolean tryATcommand(String cmd, String expected_answer, int timeout, int total_tries) {
TryAgain:
  //*************************************************************
  for (int i = 1; i <= total_tries; i++) {
    sim800l.println(cmd);
    if (getResponse(expected_answer, timeout) == true) {
      break;
    }
    else {
      Serial.print(".");
    }
  }
  //*************************************************************
}

void sendSMS(String number, String msg) {
  tryATcommand("AT+CMGS=\"" + String(number) + "\"", ">", 1000, 5);
  sim800l.print(msg);
  sim800l.write(26); // ASCII code for Ctrl+Z to send the message
  delay(1000);
  buttonPressed = true; // Mark the button as pressed to avoid sending multiple SMS
}

void buzzer(){
  // Test the buzzer by turning it on for 100 milliseconds
  tone(buzzerPin, 1000); // 1000 Hz tone
  delay(100);
  
  // Turn off the buzzer for 100 milliseconds
  noTone(buzzerPin);
  delay(100);

  // Repeat the process for a different tone
  tone(buzzerPin, 2000); // 2000 Hz tone
  delay(100);
  
  noTone(buzzerPin);
  delay(100);
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


void neo6m(){
  ss.begin(GPSBaud);
  ss.listen();
  
  unsigned long startMillis = millis();
  while (millis() - startMillis < 1000) { // Wait for 1 second
    while (ss.available()) {
      gps.encode(ss.read());
      if (gps.location.isUpdated()) {
        Serial.print("Latitude= "); 
        Serial.print(gps.location.lat(), 6);
        Serial.print(" Longitude= "); 
        Serial.println(gps.location.lng(), 6);
        return; // Exit the function once location is updated
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
  Serial.begin(9600);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT); 
  startupBuzzer(); // call this function to for sound indicating the system is up and running
}


void loop(){

  ultrasonic(trigPin1, echoPin1, buzzerPin);
  ultrasonic(trigPin2, echoPin2, buzzerPin);
  delay(100);

  if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
    state=0;
    neo6m();
    sim800l.begin(9600);
    sim800l.listen();
    Serial.println("Calling Sim800l");
    tryATcommand("ATE", "OK", 1000, 20);
    tryATcommand("AT", "OK", 1000, 20);
    tryATcommand("AT+CMGF=1", "OK", 1000, 20);
    tryATcommand("AT+CNMI=1,2,0,0,0", "OK", 1000, 20);


    String message;
    //message = "HELP ME! Current Location - Latitude: " + String(gps.location.lat(), 6) + ", Longitude: " + String(gps.location.lng(), 6);
    message = "HELP ME!!! https://www.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    Serial.println(message);
    sendSMS("447888196873", message); // Replace with the actual phone number
    Serial.println("Button Pressed");
    ss.listen();
    Serial.println("Calling GPS");
    lastBlinkTime = millis();
    ss.end();
  }

  if(state!=1){
    sim800l.end();
    buttonPressed = false;
    if (millis() - lastBlinkTime < 300000) {  // Check if 5 minutes have passed since the LEDs started blinking
      blinkLED(); // Blink LED function to blink LEDs
    } else {
      digitalWrite(ledPin1, LOW); // Turn off LEDs
      digitalWrite(ledPin2, LOW);
      state = 1; // Back to normal state
    }

  }

}



