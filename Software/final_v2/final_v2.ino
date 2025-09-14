#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(4, 5); //Tx, Rx
SoftwareSerial sim800l(7, 6); // Tx, Rx for SIM800L module
#define BUTTON_PIN 8 // Pin where the button is connected


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
      //digitalWrite(RESET_PIN, LOW);
      delay(100);
      //digitalWrite(RESET_PIN, HIGH);
      goto TryAgain;
    }
  }
  //*************************************************************
}


void setup(){
  Serial.begin(9600);
  ss.begin(GPSBaud);
  
  // pinMode(BUTTON_PIN, INPUT);

}

void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
    Serial.print("Latitude= "); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Longitude= "); 
    Serial.println(gps.location.lng(), 6);
    }
    if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
    sim800l.begin(9600);

    sim800l.listen();
    Serial.println("Calling Sim800l");
    String message;
    message = "Current Location - Latitude: " + String(gps.location.lat(), 6) + ", Longitude: " + String(gps.location.lng(), 6);
    Serial.println(message);
    sendSMS("447888196873", message); // Replace with the actual phone number
    Serial.println("Button Pressed");
    ss.listen();
    Serial.println("Calling GPS");

    // Mark the button as pressed to avoid sending multiple SMS
    buttonPressed = false;
  }

  // // Handle SMS and serial communication as before
  // while (sim800l.available()) {
  //   String response = sim800l.readString();
  //   Serial.println(response);
  //   if (response.indexOf("+CMT:") > 0) {
  //     if (response.indexOf("ledon") > 0) {
  //       digitalWrite(LED_BUILTIN, HIGH);
  //     }
  //     if (response.indexOf("ledoff") > 0) {
  //       digitalWrite(LED_BUILTIN, LOW);
  //     }
  //   }
  // }

  }
}

void sendSMS(String number, String msg) {
  tryATcommand("AT+CMGS=\"" + String(number) + "\"", ">", 1000, 5);
  sim800l.print(msg);
  sim800l.write(26); // ASCII code for Ctrl+Z to send the message
  delay(1000);
  buttonPressed = true; // Mark the button as pressed to avoid sending multiple SMS
}
