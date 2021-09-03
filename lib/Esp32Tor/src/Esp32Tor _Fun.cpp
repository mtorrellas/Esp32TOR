/**
 * 
 * */

#include "Esp32Tor.h"

#include <string>


#define NTPserver1 "time.windows.com"
#define NTPserver2 "europe.pool.ntp.org"
#define NTPserver3 "europe.pool.ntp.org"

//bool temporizador(uint16_t ms);

/*
// trim from start (in place)
static inline void ltrim(string &s)
{
  s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
}

// trim from end (in place)
static inline void rtrim(string &s)
{
  s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(string &s)
{
  ltrim(s);
  rtrim(s);
}

// trim from both ends (copying)
static inline string trimmed(string s)
{
  trim(s);
  return s;
}
*/
/*******************************************************
 * Procedimientos y funciones compartidas
 *******************************************************/
bool temporizador(uint16_t ms)
{
  static uint32_t ini = millis() - ms;
  if ((millis() - ini) >= ms)
  {
    ini += ms;
    return true;
  }

  return false;
}

bool conectar_WiFi()
{
  const char *ssid, *password;
#ifdef WFssid // WFssid y WFpassword definidos en platformio.ini
  ssid = WFssid;
  password = WFpassword;
#else
  return false;
#endif

  if (WiFi.isConnected())
    return true;

  Serial.printf("Conectando WiFi a [%s] : [%s]", ssid, password);

  if (WiFi.getMode() != WIFI_OFF)
  {
    Serial.printf("\r\nWifi no estaba apagada!\r\n");
    WiFi.persistent(true);
    WiFi.mode(WIFI_OFF);
  }

  WiFi.persistent(false);
  //WiFi.config(localIP, puerta, subred, DNSserv1, DNSserv2);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  time_t ini = millis(), act = ini;
  while (WiFi.status() != WL_CONNECTED)
  {
    act = millis();
    if (temporizador(500))
    {
      Serial.print(".");
      if (act - ini > 20000)
      {
        Serial.printf("\r\nImposible conectar Wi-Fi %s\r\n", ssid);
        return false;
      }
    }
  }

  Serial.printf("\r\nConectado a WiFi %s IP:%s  Modo: %d, en %lu segundos\r\n",
                WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.getMode(),  (act - ini) / 1000 ) ;

  return true;
}

void SNTP(timeval &tv)
{
  time_t UTC;
  //tm tml{.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 0, .tm_mon = 0, .tm_year = 0};
  tm tml{0, 0, 0, 0, 0, 0};
  //configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  tcpip_adapter_init();
  if (sntp_enabled())
  {
    sntp_stop();
  }
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char *)NTPserver1);
  sntp_setservername(1, (char *)NTPserver2);
  sntp_setservername(2, (char *)NTPserver3);
  sntp_init();

  while (tml.tm_year < (2020 - 1900))
  {
    if (temporizador(100))
    {
      Serial.print(".");
      gettimeofday(&tv, NULL);
      UTC = tv2seg(tv);
      localtime_r(&UTC, &tml);
    }
  }

  //sntp_stop(); //OJO se pone para pruebas
}


void RTC_interno(timeval * ha, double tiempo)
{
  struct timeval tv, tva;
  double n = tiempo * 27 / 3600;
  double entero, decimal;
  decimal = modf(n, &entero);

  tva.tv_sec = (time_t)entero;
  tva.tv_usec = (long)(decimal * 1000000L);

  timersub(ha, &tva, &tv);

  Serial.printf("Corregir %f, Segundos %lu, MicrSeg %lu, decimal %f\r\n", n, tva.tv_sec, tva.tv_usec, decimal);
  uint32_t ini = millis();
  while ((millis() - ini) <= 5000)
  {
    Serial.print(".");
    if (settimeofday(&tv, NULL) == 0)
    {
      break;
    }
    delay(10);
  }
}

bool torDelay(uint16_t t)
{
  uint32_t fin = millis() + t;
  while (true)
  {
    if (millis() > fin)
      return true;
  }
  return false;
}

void luzLED(uint8_t pin, uint tON, uint tOff, bool invert)
{
  static uint32_t ti = millis();
  uint tp;
  static bool estado = LOW; //Encendido es estado bajo

  if (invert)
    tp = estado ? tOff : tON;
  else
    tp = estado ? tON : tOff;

  uint32_t ta = millis();

  if (tON == 0 && tOff == 1)      //Enviando los dos parametros 0; apagmos el LED
    estado = HIGH;                //apagar LED
  else if (tON == 1 && tOff == 0) //Siempre encendido
    estado = LOW;
  else if ((ta - ti) >= tp)
  {
    //    Serial.printf("Inicio %i, Actual %i, Estado: %i, Seleción %i\r\n", ti, ta, estado, tp);
    estado = !estado;
    ti = ta;
  }
  digitalWrite(pin, estado);
}

