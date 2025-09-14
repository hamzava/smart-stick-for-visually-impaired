#include <SoftwareSerial.h>
#include <TinyGPS++.h>
SoftwareSerial gpsSerial(4,5);// Tx, Rx for GPS module
SoftwareSerial sim800l(7, 6); // Tx, Rx for SIM800L module
#define BUTTON_PIN 8 // Pin where the button is connected
const int trigPin = 3;
const int echoPin = 2;
const int buzzerPin = 9;
// Define the pin connected to the LED
const int ledPin_1 = 10;
const int ledPin_2 = 11;
boolean buttonPressed = false;

TinyGPSPlus gps;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  // Set the LED pin as an output
  pinMode(ledPin_1, OUTPUT);
  pinMode(ledPin_2, OUTPUT);
  
  Serial.begin(9600);
  gpsSerial.begin(9600);
  sim800l.begin(9600);

  
  
  // Initialize the SIM800L module
  sim800l.println("AT");
  delay(1000);
  sim800l.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);

}

void loop() {
  
  if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
    Serial.println("Button Pressed");
    // Read GPS data
    buttonPressed = true;
    String data = "";
    gpsSerial.listen();
    Serial.println("Called GPS");
    delay(5000);
    while (gpsSerial.available())
    {
      gps.encode(gpsSerial.read());
      if (gps.location.isUpdated()) 
      {
        Serial.print("Latitude= ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(" Longitude= ");
        Serial.println(gps.location.lng(), 6);
      }
    }

    String gpsData = "Current Location - Latitude: " + String(gps.location.lat(), 6) + ", Longitude: " + String(gps.location.lng(), 6);
      // Send SMS with GPS data
    sim800l.listen();
    
    Serial.println("Called Sim");
    delay(1000);
    //sendSMS(gpsData);
    delay(660000); // Delay for 1 minute before sending the next SMS
    digitalWrite(ledPin_1, HIGH);
    delay(500);
    digitalWrite(ledPin_1, LOW);
    digitalWrite(ledPin_2, HIGH);
    delay(500);
    digitalWrite(ledPin_2, LOW);
    }

  //   // Ultrasonic Sensor
  // long duration, distance;
  
  // // Trigger ultrasonic sensor
  // digitalWrite(trigPin, LOW);
  // delayMicroseconds(2);
  // digitalWrite(trigPin, HIGH);
  // delayMicroseconds(10);
  // digitalWrite(trigPin, LOW);
  
  // // Read the echo pulse duration
  // duration = pulseIn(echoPin, HIGH);

  // // Calculate distance in centimeters
  // distance = duration * 0.034 / 2;

  // // Print the distance on the serial monitor
  // Serial.print("Distance: ");
  // Serial.print(distance);
  // Serial.println(" cm");
  // // Check if an object is in front (adjust the distance threshold as needed)
  // if (distance < 30) {
  //   // Beep the buzzer
  //   digitalWrite(buzzerPin, HIGH);
  //   delay(500); // Beep for 500 milliseconds
  //   digitalWrite(buzzerPin, LOW);
  // }

  // delay(50); // Wait for 1 second before the next measurement
  

}

void sendSMS(String message) {
// Replace 'PHONE_NUMBER' with the destination phone number
  sim800l.print("AT+CMGS=\"447888196873\"\r");
  delay(1000);
  sim800l.print(message);
  delay(1000);
  sim800l.write(26); // ASCII code for Ctrl+Z
  delay(1000);
}