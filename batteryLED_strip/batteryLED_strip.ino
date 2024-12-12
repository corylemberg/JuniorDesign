#include <FastLED.h>

#define LED_PIN   12
#define NUM_LEDS  12


CRGB leds[NUM_LEDS];
float input;

void setup() {
  input = 0;
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  float input = analogRead(A1) * 5.0/1023;
  int charge = (input - 3.75) / 0.75 * 12.0;
  if(charge <= 0) {
    charge = 0;
  }
  else if(charge >= 12) {
    charge = 12;
  }
  Serial.println(charge);
  delay(50);
  for (int i = 0; i <= NUM_LEDS-1; i++) {
      if (i <= charge) {
        leds[i] = CRGB (80 - (charge * 6), charge * 6, 0);
      }
      else leds[i] = CRGB (0, 0, 0);
    FastLED.show();
  }
}

