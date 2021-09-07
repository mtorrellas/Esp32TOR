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

void fechaTOR::luzLED()
{
  bool est = cambioEstado(LED_On, LED_Off);
  if( LED_estado !=  est)
    LED_estado = est;
    this->LED(LED_estado);
}

void fechaTOR::actualizar(const char *msg, const char * FL)
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
  Serial.printf("%s: UTC %lu-%s%s", msg, this->UTC, this->cFechaLarga, FL);
#endif
}

bool fechaTOR::update(int16_t seg)
{
   if(this->LED != nullptr)
    this->luzLED();

  if (temporizador(1000))
  {
    this->actualizar("Ahora", "\t");
    static int p =  seg-this->UTC % seg;
    if (p == 0)
      p = seg;
    Serial.printf("\tPasada %d\r\n", p-- );
    
    if ( p == 0)
    {
      this->suspender();
    }
  }

  return false;
}

void fechaTOR::begin( void(*led)(bool) )
{
  this->LED = led;

  Serial.begin(115200);
  Serial.println();
  Serial.println("*******************************************************************************");
  
  if (tv2seg(RTC_iniciado) == 0)
  {
    if ( conectar_WiFi() )
      SNTP(RTC_iniciado);
    //this->actualizar("Reloj ajustado");

    RTC_error.tv_sec = 0;
    RTC_error.tv_usec = 0;
    RTC_reinicio = RTC_iniciado;
    RTC_despertar = RTC_iniciado;
  }
  Serial.printf("Inicio UTC%lu, Reiniciado: UTC%lu, Reactivar: UTC%lu\r\n", 
         tv2seg(RTC_iniciado), tv2seg(RTC_reinicio), tv2seg(RTC_despertar) );
}

void fechaTOR::activar()
{
  Serial.println("===============================================================================");

  esp_sleep_wakeup_cause_t causaACTV = esp_sleep_get_wakeup_cause();
  Serial.println(DESPdatos[causaACTV]);

  this->actualizar("Sistema Activado");

  switch (causaACTV)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    RTC_reinicio = RTC_actual;

    //RTC_despertar.tv_sec += SLEEP_INTERVALO;
    timersub( &RTC_reinicio, &RTC_despertar, &RTC_error );
    settimeofday(&RTC_despertar, NULL);

    this->actualizar("Fecha INTERNA", "\t");
    Serial.printf("Error %f seg.\r\n", tv2seg_f( RTC_error) );
    break;
  default: 
    break;
  }
  Serial.println("===============================================================================");
}

void fechaTOR::suspender()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  FT.actualizar("\r\nDesconectar sistema");
  // RTC_despertar.tv_sec = this->UTC - this->UTC % SLEEP_INTERVALO;
  // RTC_despertar.tv_usec = 0;
  RTC_despertar.tv_sec = SLEEP_INTERVALO + this->UTC - this->UTC % SLEEP_INTERVALO;
  RTC_despertar.tv_usec = 0;



  if ( RTC_despertar.tv_sec % 3600  == 0)
  {
    RTC_iniciado.tv_sec = 0;
    RTC_iniciado.tv_usec = 0;
  }

  struct timeval tv;
  timersub(&RTC_despertar, &RTC_actual, &tv);

  float tiempo = tv2microseg(tv);
  float error =  tiempo * 15 / 3600; //tv2microseg(RTC_error); 

  // Serial.printf( "Suspendido durante %.3f segundos + %.3f de error, Actual: UTC%lu, Reinicio: UTC%lu\r\n", 
  //             tiempo / 1000000, error / 1000000, tv2seg(RTC_actual), tv2seg(RTC_despertar) );
  Serial.printf( "Suspendido durante %.3f segundos + %.3f de error, Actual: UTC%lu, Reinicio: UTC%lu\r\n", 
              tv2seg_f(tv), error / 1000000, tv2seg(RTC_actual), tv2seg(RTC_despertar) );

  Serial.println("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");

  esp_deep_sleep( round(tiempo + error) );
}

fechaTOR FT;