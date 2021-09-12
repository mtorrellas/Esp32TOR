#include <M5StickC.h>
#include "Esp32Tor.h"

// #define VERSION '"3.0.1"'
// #define PLACA '"M5StickC"'
// #define _DEBUG_
// #define INTERVALO 300
// #define WFssid '"famitor_IoT"'
// #define WFpassword '"tarazona"'

void LCD_reloj();
void LCD_inicio(const char *TIT = "Esp32Tor");
bool torDelay(uint16_t t);

void luz(bool st)
{
  digitalWrite(M5_LED, !st);
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
  RTC_DateTypeDef dt;
  RTC_TimeTypeDef tt;

  dt.Date = FT.dia;
  dt.Month = FT.mes;
  dt.Year = FT.anio;

  tt.Hours = FT.hora;
  tt.Minutes = FT.minuto;
  tt.Seconds = FT.segundo;

  // M5.Rtc.SetData(&dt);
  // M5.Rtc.SetTime(&tt);

  Serial.printf("Importado desde \"fechaTOR\": %02d-%02d-%04d %02d:%02d:%02d\r\n",
       dt.Date, dt.Month, dt.Year, tt.Hours, tt.Minutes, tt.Seconds );
}

void dormir(uint64_t tiempo)
{
  //M5.Axp.SetLDO2(false);
  //M5.Axp.SetLDO3(false);
  M5.Axp.DeepSleep( tiempo );  
}

void setup()
{
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);  
  M5.begin();

  FT.begin(luz, nullptr, TF2reloj, dormir);
  FT.setLED(50, 950);

  LCD_inicio();
}

void loop()
{
  if(FT.update(20) )
    LCD_reloj();
}

void LCD_inicio(const char *TIT)
{
  M5.Axp.ScreenBreath(9);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(TFT_ORANGE);
  M5.Lcd.setTextColor(TFT_BLUE);

  uint8_t tempdatum = M5.Lcd.getTextDatum();
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString(TIT, 80, 40, 4);

  M5.Lcd.setTextDatum(tempdatum);
}

void LCD_reloj()
{
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(TFT_NAVY);

  uint8_t tempdatum = M5.Lcd.getTextDatum();

  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextDatum(TC_DATUM);

  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.drawString(FT.cFecha, 80, 3, 1);

  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextColor(TFT_GREENYELLOW);
  M5.Lcd.drawString(FT.cDia, 80, 39, 4);

  M5.Lcd.setTextDatum(BC_DATUM);
  M5.Lcd.setTextColor(TFT_ORANGE);
  M5.Lcd.drawString(FT.cHora, 80, 78, 4);

  M5.Lcd.setTextDatum(tempdatum);
}

void LCD_reloja()
{
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(TFT_NAVY);

  uint8_t tempdatum = M5.Lcd.getTextDatum();

  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextDatum(TC_DATUM);

  //sprintf(buf, "%02i-%02i-%04i", _fecha.dia, _fecha.mes, _fecha.anio);
  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.drawString(FT.cFecha, 80, 5, 2);

  M5.Lcd.setTextColor(TFT_GREENYELLOW);
  M5.Lcd.drawString(FT.cDia, 80, 28, 4);

  M5.Lcd.setTextDatum(BC_DATUM);
  //sprintf(buf, "%02i:%02i:%02i", _fecha.hora24h, _fecha.minuto, _fecha.segundo);
  M5.Lcd.setTextColor(TFT_ORANGE);
  M5.Lcd.drawString(FT.cHora, 80, 78, 4);

  M5.Lcd.setTextDatum(tempdatum);
}