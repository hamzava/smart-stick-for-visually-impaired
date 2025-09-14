// Define the pins for the LEDs
const int ledPin1 = 10;
const int ledPin2 = 11;

// Define the time interval for blinking in milliseconds
const unsigned long blinkInterval = 1000; // Blink interval in milliseconds

unsigned long previousMillis = 0; // Stores the last time LED was updated

int ledState = LOW; // Tracks the current state of the LED

void setup() {
  // Initialize the digital pins as outputs
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}

void loop() {
  // Get the current time
  unsigned long currentMillis = millis();

  // Check if it's time to blink the LED
  if (currentMillis - previousMillis >= blinkInterval) {
    // Save the last time the LED blinked
    previousMillis = currentMillis;

    // Toggle the LED state
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // Write the LED state to the pins
    digitalWrite(ledPin1, ledState);
    digitalWrite(ledPin2, !ledState); // Invert the state for the second LED
  }
}
