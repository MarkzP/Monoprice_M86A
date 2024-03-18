#include "vianet.h"
#include "monoprice.h"

#define VIANET_TE_PIN  2
#define TRIGGER_ZONE_1 13

Vianet vianet;
Monoprice monoprice;

void setup()
{
  pinMode(TRIGGER_ZONE_1, OUTPUT);
  delay(200);

  vianet.begin(&Serial1, VIANET_TE_PIN);
  monoprice.begin(&Serial, &vianet);
}

void loop()
{
  monoprice.update();

  // Zone Activated Trigger example:
  // Unlike the Monoprice amp (& older Elan S86a units), M86a's are lacking 12v trigger outputs on zones.
  // It can be emulated with a GPIO pin - ex. this can be used to turn on a subwoofer amp if Zone 1 is active.
  digitalWrite(TRIGGER_ZONE_1, vianet.getSource(1) != 0 ? HIGH : LOW);
}
