#include <Arduino.h>

#define RELAY_PIN D0

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off initially
}

void loop()
{
  
}