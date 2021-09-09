#include "Esp32Tor.h"

void luz(bool st)
{
  digitalWrite(LED_BUILTIN, st);
}

void reloj2TF()
{
  FT.intercambio.segundo  = 30;
  FT.intercambio.minuto   = 1;
  FT.intercambio.hora     = 12;
  FT.intercambio.dia      = 1;
  FT.intercambio.mes      = 1;
  FT.intercambio.anio     = 2000;
}

void TF2reloj()
{
  Serial.print("Para ajustar reloj placa: ");
  Serial.println(FT.cFechaLarga);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  FT.begin(luz, nullptr, TF2reloj );
  Serial.printf("\r\nEJEMPLO USO LIBRERIA \"Esp32Tor\" Version: %s\r\n", VERSION);
  FT.activar();
  FT.setLED(50, 950);
}

void loop()
{
  FT.update(25);
}