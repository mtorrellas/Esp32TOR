#include "Esp32Tor.h"

void luz(bool st)
{
  digitalWrite(LED_BUILTIN, st);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  FT.begin(luz);
  FT.activar();
}

void loop()
{
  FT.update(25);
}