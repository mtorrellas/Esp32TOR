#include "Esp32Tor.h"
//#include "Esp32Tor.hpp"

void setup()
{
  FT.begin();
  Serial.println("===============================================================================");
  FT.activar();
  Serial.println("===============================================================================");
}

void loop()
{

  FT.update();
}