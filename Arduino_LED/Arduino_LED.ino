#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    60
#define BRIGHTNESS  128
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB // Falls deine Farben (Rot/Grün) vertauscht sind, hier GRB nutzen

CRGB leds[NUM_LEDS];

enum Mode {
  MODE_OFF = 0,
  MODE_STATIC_COLOR = 1,
  MODE_RAINBOW = 2,
  MODE_BREATHE = 3,
  MODE_RUNNING = 4
};

Mode currentMode = MODE_OFF;
uint8_t r = 255, g = 255, b = 255;
uint8_t brightness = BRIGHTNESS;
uint8_t rainbowHue = 0;
uint8_t breatheVal = 0;
int8_t breatheDir = 1;

// NEU: Variable für die Geschwindigkeit (Delay in Millisekunden)
// Wir mappen den Wert 0-255 vom Slider auf ein sinnvolles Delay (z.B. 5ms bis 100ms)
uint16_t animDelay = 20; 

String inputBuffer = "";

void setup() {
  Serial.begin(115200);   // USB Debugging
  Serial1.begin(115200);  // Kommunikation mit dem Pi
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  Serial1.println("READY");
}

void loop() {
  while (Serial1.available()) { processChar(Serial1.read()); }
  while (Serial.available())  { processChar(Serial.read()); }

  switch (currentMode) {
    case MODE_OFF:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      break;
    case MODE_STATIC_COLOR:
      fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
      break;
    case MODE_RAINBOW:
      fill_rainbow(leds, NUM_LEDS, rainbowHue++, 7);
      break;
    case MODE_BREATHE:
      breatheVal += breatheDir * 2;
      if (breatheVal >= 250) breatheDir = -1;
      if (breatheVal <= 5)   breatheDir = 1;
      fill_solid(leds, NUM_LEDS, CRGB(map(breatheVal, 0, 255, 0, r), map(breatheVal, 0, 255, 0, g), map(breatheVal, 0, 255, 0, b)));
      break;
    case MODE_RUNNING:
      fadeToBlackBy(leds, NUM_LEDS, 20); // Schöner Schweif-Effekt
      leds[rainbowHue % NUM_LEDS] = CRGB(r, g, b);
      rainbowHue++;
      break;
  }
  
  FastLED.show();
  delay(animDelay); // Nutzt jetzt die variable Geschwindigkeit
}

void processChar(char c) {
  if (c == '\n' || c == '\r') {
    if (inputBuffer.length() > 0) {
      handleCommand(inputBuffer);
      inputBuffer = "";
    }
  } else {
    inputBuffer += c;
  }
}

void handleCommand(String cmd) {
  cmd.trim();
  
  // Feedback an Pi senden
  Serial1.print("OK: "); Serial1.println(cmd);

  if (cmd == "s" || cmd == "0") {
    currentMode = MODE_OFF;
  } 
  else if (cmd.startsWith("m:")) {
    currentMode = (Mode)cmd.substring(2).toInt();
  } 
  else if (cmd.startsWith("c:")) {
    String val = cmd.substring(2);
    int c1 = val.indexOf(',');
    int c2 = val.lastIndexOf(',');
    if (c1 > 0 && c2 > c1) {
      r = val.substring(0, c1).toInt();
      g = val.substring(c1 + 1, c2).toInt();
      b = val.substring(c2 + 1).toInt();
      currentMode = MODE_STATIC_COLOR;
    }
  } 
  else if (cmd.startsWith("b:")) {
    brightness = constrain(cmd.substring(2).toInt(), 0, 255);
    FastLED.setBrightness(brightness);
  }
  // NEU: Geschwindigkeit verarbeiten
  else if (cmd.startsWith("v:")) {
    uint8_t rawSpeed = cmd.substring(2).toInt();
    // Umkehr-Logik: Hoher Slider-Wert (255) soll SCHNELL sein (kleines Delay)
    // Wir mappen 0-255 auf 100ms bis 5ms Delay
    animDelay = map(rawSpeed, 0, 100, 150, 2);
  }
}