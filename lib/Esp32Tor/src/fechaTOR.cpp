#include "Esp32Tor.h"


const char *DESPdatos[] =
    {"Inicio del dispositivo por motivo desconicido",
     "No es una causa de activación, se usa para deshabilitar todas las fuentes de activación",
     "Despertar ocasionado por señal externa RTC_IO",
     "Despertar ocasionado por señal externa RTC_CNTL",
     "Despertar ocasionado por timer",
     "Despertar ocasionado por touchpad",
     "Despertar ocasionado por ULP program",
     "Despertar ocasionado por GPIO (light sleep only)",
     "Despertar ocasionado por UART (light sleep only)"};

RTC_DATA_ATTR timeval RTC_iniciado = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_reinicio = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_actual = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_despertar = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_error= {.tv_sec = 0, .tv_usec = 0};

fechaTOR::fechaTOR()
{
  setenv("TZ", this->CET, 1);
  tzset();
}

fechaTOR::~fechaTOR() {}



void fechaTOR::actualizar(const char *msg)
{
  struct tm tml;
  gettimeofday(&RTC_actual, NULL);

  this->UTC = tv2seg(RTC_actual);

  localtime_r(&this->UTC, &tml);

  this->segundo = tml.tm_sec;
  this->minuto = tml.tm_min;
  this->hora = tml.tm_hour;
  this->hora12h = (tml.tm_hour > 12) ? tml.tm_hour - 12 : tml.tm_hour;
  this->diaSemana = tml.tm_wday;
  this->dia = tml.tm_mday;
  this->mes = tml.tm_mon + 1;
  this->anio = tml.tm_year + 1900;
  this->cDia = NombreDia[tml.tm_wday];
  this->cMes = NombreMes[tml.tm_mon];
  strftime(this->cHora, 10, "%H:%M:%S", &tml);
  strftime(this->cFecha, 10, "%d-%m-%Y", &tml);
  sprintf(this->cFechaCorta, "%02i %s %04i",
          this->dia, this->cMes, this->anio);
  sprintf(this->cFechaLarga, "%s, %02i de %s de %04i, %s",
          this->cDia, this->dia, this->cMes, this->anio, this->cHora);
#ifdef _FT_DEBUG
  Serial.printf("%s: UTC %lu-%s\r\n", msg, this->UTC, this->cFechaLarga);
#endif
}

bool fechaTOR::update(uint16_t seg)
{
  static uint16_t pasada = 0 ;
  if (temporizador(1000))
  {
    Serial.printf("Pasada % 3d-", ++pasada );
    this->actualizar("Ahora");
    if ( pasada >= seg)
    {
      //pasada = 0;
      this->suspender();
    }
  }

  return false;
}

void fechaTOR::begin(  bool (*ext)(uint16_t s))
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("*******************************************************************************");
  
  if (tv2seg(RTC_iniciado) == 0)
  {
    if ( conectar_WiFi() )
      SNTP(RTC_iniciado);

    this->actualizar("Reloj ajustado");

    RTC_reinicio = RTC_iniciado;
    RTC_despertar.tv_sec = this->UTC + SLEEP_INTERVALO - this->UTC % SLEEP_INTERVALO;
    RTC_despertar.tv_usec = 0;
    uint16_t susp = tv2seg(RTC_despertar) - tv2seg(RTC_reinicio);
    if ( susp < 20 )
      RTC_despertar.tv_sec += SLEEP_INTERVALO;
  }
  Serial.printf("Inicio UTC%lu, Reiniciado: UTC%lu, Reactivar: UTC%lu\r\n", 
         tv2seg(RTC_iniciado), tv2seg(RTC_reinicio), tv2seg(RTC_despertar) );
}

void fechaTOR::suspender()
{
  WiFi.disconnect(true);
  FT.actualizar("\r\nDesconectar sistema");
  struct timeval tv;

  timersub(&RTC_despertar, &RTC_actual, &tv);
  time_t tiempo = tv2microseg(tv);
  float error = (float)tiempo * 24 / 3600;
  Serial.printf( "Suspendido durante %f segundos + %f de error, Actual: UTC%lu, Reinicio: UTC%lu\r\n", 
              (float)tiempo / 1000000, error / 1000000, tv2seg(RTC_actual), tv2seg(RTC_despertar) );

  Serial.println("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");


  esp_deep_sleep( tiempo + error );
}

void fechaTOR::activar()
{
  esp_sleep_wakeup_cause_t causaACTV = esp_sleep_get_wakeup_cause();
  Serial.println(DESPdatos[causaACTV]);

  this->actualizar("Sistema Activado");

  switch (causaACTV)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    RTC_reinicio = RTC_actual;
    settimeofday(&RTC_despertar, NULL);
    RTC_despertar.tv_sec += SLEEP_INTERVALO;
    //RTC_interno(&RTC_actual, SLEEP_INTERVALO);

    this->actualizar("Fecha INTERNA");
    break;
  default: 
    break;
  }
  
}

fechaTOR FT;