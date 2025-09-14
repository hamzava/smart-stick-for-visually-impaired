const int buttonPin = 5; // Pin number where the button is connected

void setup() {
  pinMode(buttonPin, INPUT); // Set the button pin as INPUT
  Serial.begin(9600); // Initialize serial communication for debugging
}

void loop() {
  int buttonState = digitalRead(buttonPin); // Read the state of the button (HIGH or LOW)

  if (buttonState == HIGH) {
    Serial.println("Button is pressed (I LOVE MY QUEEN SENA)");
  } else {
    Serial.println("Button is not pressed (LOW)");
  }

  delay(500); // Add a small delay to avoid rapid serial prints
}
