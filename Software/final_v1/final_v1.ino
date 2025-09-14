#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial gpsSerial(4, 3); // Tx, Rx for GPS module
SoftwareSerial sim800l(7, 6); // Tx, Rx for SIM800L module
#define BUTTON_PIN 5 // Pin where the button is connected


boolean buttonPressed = false;

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
    if (i == total_tries) {
      Serial.println("Failed! Resetting the Module");
     
      delay(100);
     
      goto TryAgain;
    }
  }
  //*************************************************************
}

void setup() {
 
  pinMode(BUTTON_PIN, INPUT);

  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
  

  tryATcommand("ATE", "OK", 1000, 20);
  tryATcommand("AT", "OK", 1000, 20);
  tryATcommand("AT+CMGF=1", "OK", 1000, 20);
  tryATcommand("AT+CNMI=1,2,0,0,0", "OK", 1000, 20);
}

void loop() {
  // GPS tracking
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      Serial.print("Latitude= ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= ");
      Serial.println(gps.location.lng(), 6);
    }
  }

  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
    // Disable interrupts to stop communication with the GPS module
    noInterrupts();
    sim800l.begin(9600);
    delay(5000);

    // Button is pressed, send SMS
    sendSMS("447888196873", "Button pressed!"); // Replace with the actual phone number
    Serial.println("Button Pressed");

    // Mark the button as pressed to avoid sending multiple SMS
    buttonPressed = true;

    // Enable interrupts to resume communication with the GPS module
    interrupts();
  }

  // Handle SMS and serial communication as before
  while (sim800l.available()) {
    String response = sim800l.readString();
    Serial.println(response);
    if (response.indexOf("+CMT:") > 0) {
      if (response.indexOf("ledon") > 0) {
        digitalWrite(LED_BUILTIN, HIGH);
      }
      if (response.indexOf("ledoff") > 0) {
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }

  while (Serial.available()) {
    sim800l.println(Serial.readString());
  }
}

// Function to send SMS
void sendSMS(String number, String msg) {
  tryATcommand("AT+CMGS=\"" + String(number) + "\"", ">", 1000, 5);
  sim800l.print(msg);
  sim800l.write(26); // ASCII code for Ctrl+Z to send the message
  delay(1000);
  buttonPressed = true; // Mark the button as pressed to avoid sending multiple SMS
}

