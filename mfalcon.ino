#include <FastLED.h>

// Define the pins for the 4 regular LEDs
const int LED_PINS[] = {2, 3, 4, 5};  // Pins for the 4 LEDs
const int NUM_LEDS = 4;

// Define the pin for the switch
const int SWITCH_PIN = 6;

// Define the data pin for the NeoPixel strip
const int NEOPIXEL_PIN = 7;
const int NUM_NEOPIXELS = 300;

// Create an array for the NeoPixel LEDs
CRGB neopixels[NUM_NEOPIXELS];

// Variables to track states
bool switchPrevState = false;  // Previous state of the switch
bool sequenceRunning = false;  // Is the LED sequence running?
unsigned long sequenceStartTime = 0;  // When did the sequence start?
int currentLed = -1;  // Which LED in the sequence is on (-1 means none)
bool chasingActive = false;  // Is the chasing effect active?
unsigned long chasingStartTime = 0;  // When did the chasing start?

void setup() {
  // Set up the regular LED pins as outputs
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);  // Start with LEDs off
  }
  
  // Set up the switch pin as input with pull-up resistor
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  
  // Initialize FastLED with our NeoPixel strip
  FastLED.addLeds<WS2812B, NEOPIXEL_PIN, GRB>(neopixels, NUM_NEOPIXELS);
  FastLED.setBrightness(50);  // Set brightness to 1/5 (0-255)
  
  // Clear all NeoPixels
  fill_solid(neopixels, NUM_NEOPIXELS, CRGB::Black);
  FastLED.show();
  
  // Initialize serial for debugging
  Serial.begin(9600);
  Serial.println("LED Controller initialized");
}

void loop() {
  // Read the current state of the switch (HIGH = not pressed, LOW = pressed)
  bool switchPressed = (digitalRead(SWITCH_PIN) == LOW);
  
  // Detect switch pressed event
  if (switchPressed && !switchPrevState) {
    // Switch was just pressed
    startLedSequence();
  }
  
  // Detect switch released event
  if (!switchPressed && switchPrevState) {
    // Switch was just released
    handleSwitchReleased();
  }
  
  // Update current LED state if sequence is running
  if (sequenceRunning) {
    updateLedSequence();
  }
  
  // Update chasing effect if active
  if (chasingActive) {
    updateChasing();
  }
  
  // Save the current switch state for the next loop
  switchPrevState = switchPressed;
  
  // Small delay to avoid bouncing
  delay(10);
}

void startLedSequence() {
  if (chasingActive) {
    // If chasing is active, ignore the switch press
    Serial.println("Chasing effect is active, ignoring switch press");
    return;
  }
  
  sequenceRunning = true;
  sequenceStartTime = millis();
  currentLed = -1;
  
  // Turn off all LEDs to start fresh
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
  
  // Turn off any NeoPixels that might be on
  fill_solid(neopixels, NUM_NEOPIXELS, CRGB::Black);
  FastLED.show();
  
  Serial.println("Started LED sequence");
}

void updateLedSequence() {
  // Calculate how many LEDs should be on based on elapsed time
  unsigned long elapsed = millis() - sequenceStartTime;
  int ledsToLight = min(elapsed / 1000, (unsigned long)NUM_LEDS);
  
  // If we need to light a new LED
  if (ledsToLight > currentLed) {
    currentLed = ledsToLight;
    
    // Turn on the current LED
    if (currentLed < NUM_LEDS) {
      digitalWrite(LED_PINS[currentLed], HIGH);
      Serial.print("Turned on LED ");
      Serial.println(currentLed);
    }
    
    // If all LEDs are now lit, sequence is complete but stays active
    if (currentLed >= NUM_LEDS - 1) {
      Serial.println("All LEDs are now on");
    }
  }
}

void handleSwitchReleased() {
  // If sequence was running
  if (sequenceRunning) {
    sequenceRunning = false;
    
    turnOffAllLeds();

    // Check if all LEDs were on when switch was released
    if (currentLed >= NUM_LEDS - 1) {
      // Start the chasing effect
      startChasing();
    }
  }
}

void turnOffAllLeds() {
  // Turn off all regular LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
  
  // Turn off all NeoPixels
  fill_solid(neopixels, NUM_NEOPIXELS, CRGB::Black);
  FastLED.show();
  
  Serial.println("All LEDs turned off");
}

void startChasing() {
  chasingActive = true;
  chasingStartTime = millis();
  
  Serial.println("Started chasing effect");
}

void updateChasing() {
  // Check if the 30 seconds have passed
  if (millis() - chasingStartTime > 30000) {
    chasingActive = false;
    turnOffAllLeds();
    Serial.println("Chasing effect completed");
    return;
  }
  
  // Create a moving pattern with 5 LEDs lit at a time
  int position = (millis() / 5) % NUM_NEOPIXELS; // Controls the speed
  
  // Clear all NeoPixels first
  fill_solid(neopixels, NUM_NEOPIXELS, CRGB::Black);
  
  // Light up 5 consecutive LEDs
  for (int i = 0; i < 15; i++) {
    int pixelPos = (position + i) % NUM_NEOPIXELS;
    neopixels[pixelPos] = CRGB::Blue;  // You can change the color
  }
  
  // Show the updated NeoPixels
  FastLED.show();
}
