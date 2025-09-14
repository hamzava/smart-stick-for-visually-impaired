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
long duration;
int distance;
unsigned long lastBlinkTime = 0;
int ledState = LOW;



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

void ultra(){
// Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance from the object = ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 15) {
  buzzer();
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

void blinkLEDs(int blinkDuration) {
  unsigned long startTime = millis(); // Get the current time in milliseconds
  unsigned long endTime = startTime + (10 * 60 * 1000); // Calculate end time (10 minutes from start)

  while (millis() < endTime) { // Run loop until 10 minutes have passed
    digitalWrite(ledPin1, HIGH); // Turn the first LED on
    digitalWrite(ledPin2, LOW);  // Turn the second LED on
    delay(blinkDuration);        // Wait for the specified duration
    digitalWrite(ledPin1, LOW);  // Turn the first LED off
    digitalWrite(ledPin2, HIGH); // Turn the second LED off
    delay(blinkDuration);        // Wait for the specified duration
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
  
}

void loop(){

  ultra();

  if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
    state=0;
    neo6m();
    sim800l.begin(9600);
    sim800l.listen();
    Serial.println("Calling Sim800l");
    String message;
    //message = "HELP ME! Current Location - Latitude: " + String(gps.location.lat(), 6) + ", Longitude: " + String(gps.location.lng(), 6);
    message = "HELP ME!!! https://www.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    Serial.println(message);
    sendSMS("447888196873", message); // Replace with the actual phone number
    Serial.println("Button Pressed");
    ss.listen();
    Serial.println("Calling GPS");
    lastBlinkTime = millis();
  }

  if(state!=1){
    // Check if 10 minutes have passed since the LEDs started blinking
    if (millis() - lastBlinkTime < 600000) {
      // Blink the LEDs every 100 milliseconds
      // Blink the LEDs
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, LOW);
      delay(50); // Adjust this value as desired
      digitalWrite(ledPin1, LOW);
      digitalWrite(ledPin2, HIGH);
      delay(50); // Adjust this value as desired
    }
    else{
      digitalWrite(ledPin1, LOW);
      digitalWrite(ledPin2, LOW);
    }
  }
}



