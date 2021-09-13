#include "Esp32Tor.h"

const char *DESPdatos[] =
    {"Activación no causado por salir de suspensión",
     "No es una causa de activación, se usa para deshabilitar todas las fuentes de activación",
     "Activación ocasionada por señal externa RTC_IO",
     "Activación ocasionada por señal externa RTC_CNTL",
     "Activación ocasionada por timer",
     "Activación ocasionada por touchpad",
     "Activación ocasionada por ULP program",
     "Activación ocasionada por GPIO (light sleep only)",
     "Activación ocasionada por UART (light sleep only)"};
esp_sleep_wakeup_cause_t causaACTV;
RTC_DATA_ATTR timeval RTC_iniciado = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_reinicio = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_actual = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_despertar = {.tv_sec = 0, .tv_usec = 0};
RTC_DATA_ATTR timeval RTC_error = {.tv_sec = 0, .tv_usec = 0};

fechaTOR::fechaTOR()
{
  setenv("TZ", this->CET, 1);
  tzset();
}

fechaTOR::~fechaTOR(){};

void fechaTOR::setLED(uint on, uint off)
{
  this->LED_On = on;
  this->LED_Off = off;
}

void fechaTOR::luzLED()
{
  bool est = cambioEstado(LED_On, LED_Off);
  if (LED_estado != est)
    LED_estado = est;
  this->LED(LED_estado);
}

bool fechaTOR::importar()
{
  if (entrada == nullptr)
    return false;

  struct tm ti;
  timeval tv;
  this->entrada();
  ti.tm_sec = intercambio.segundo;
  ti.tm_min = intercambio.minuto;
  ti.tm_hour = intercambio.hora;
  ti.tm_mday = intercambio.dia;
  ti.tm_mon = intercambio.mes - 1;
  ti.tm_year = intercambio.anio - 1900;

  tv.tv_sec = mktime(&ti);
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  this->actualizar("Reloj importado");
  RTC_reinicio = RTC_actual;
  RTC_despertar = RTC_iniciado;
  RTC_error.tv_sec = 0;
  RTC_error.tv_usec = 0;
  
  return true;
}

void fechaTOR::actualizar(const char *msg, const char *FL)
{
  struct tm tml;
  gettimeofday(&RTC_actual, NULL);

  this->T_UTC = tv2seg(RTC_actual);

  localtime_r(&this->T_UTC, &tml);

  this->T_segundo = tml.tm_sec;
  this->T_minuto = tml.tm_min;
  this->T_hora = tml.tm_hour;
  this->T_hora12h = (tml.tm_hour > 12) ? tml.tm_hour - 12 : tml.tm_hour;
  this->T_diaSemana = tml.tm_wday;
  this->T_dia = tml.tm_mday;
  this->T_mes = tml.tm_mon + 1;
  this->T_anio = tml.tm_year + 1900;
  this->T_cDia = NombreDia[tml.tm_wday];
  this->T_cMes = NombreMes[tml.tm_mon];
  strftime(this->T_cHora, 10, "%H:%M:%S", &tml);
  strftime(this->T_cFecha, 12, "%d-%m-%Y", &tml);
  sprintf(this->T_cFechaCorta, "%02i %s %04i",
          this->T_dia, this->T_cMes, this->T_anio);
  sprintf(this->T_cFechaLarga, "%s, %02i de %s de %04i, %s",
          this->T_cDia, this->T_dia, this->T_cMes, this->T_anio, this->T_cHora);

  // intercambio.segundo   = this->segundo;
  // intercambio.minuto    = this->minuto;
  // intercambio.hora      = this->hora;
  // intercambio.hora12h   = this->hora12h;
  // intercambio.diaSemana = this->diaSemana;
  // intercambio.dia       = this->dia;
  // intercambio.mes       = this->mes;
  // intercambio.anio      = this->anio;
  // intercambio.cDia      = this->cDia;
  // intercambio.cMes      = this->cMes;

#ifdef _TOR_DEBUG
  Serial.printf("%s: UTC %lu-%s%s", msg, this->T_UTC, this->T_cFechaLarga, FL);
#endif
}

bool fechaTOR::update(int16_t seg)
{
  static int p = seg;
  if (this->LED != nullptr)
    this->luzLED();

  if (temporizador(1000))
  {
    this->actualizar("Ahora", "\t");
    //static int p = seg - this->T_UTC % seg;
  
    if (p == 0)
    {
      p = seg;
      this->suspender();
    }
    Serial.printf("\tPasada %d\r\n", p--);
    return true;
  }

  return false;
}

void fechaTOR::inicio()
{
    if (conectar_WiFi())
    {
      SNTP(RTC_iniciado);
      this->actualizar("Reloj Ajustado");
      RTC_reinicio = RTC_iniciado;
      RTC_despertar = RTC_iniciado;
      RTC_error.tv_sec = 0;
      RTC_error.tv_usec = 0;
      if (salida != nullptr)
        salida();
    }
    else if (!this->importar())
      Serial.println("Imposible ajustar el reloj del sistema");

}

void fechaTOR::begin(void (*led)(bool), void (*ent)(), void (*sal)(), void (*dmr)(uint64_t) )
{
  this->LED = led;
  this->entrada = ent;
  this->salida = sal;
  this->dormir = dmr;

  Serial.begin(115200);
  Serial.flush();
  Serial.println("Iniciando Esp32TOR. . .");
  Serial.println("*******************************************************************************");

  Serial.printf("LIBRERIA [Esp32Tor] Version: %s\r\n", TOR_VERSION);

  Serial.printf("Inicio UTC%lu, Reiniciado: UTC%lu, Reactivar: UTC%lu\r\n",
                tv2seg(RTC_iniciado), tv2seg(RTC_reinicio), tv2seg(RTC_despertar));

  this->activar();
}

void fechaTOR::activar()
{
  Serial.println("===============================================================================");

  causaACTV = esp_sleep_get_wakeup_cause();
  Serial.println(DESPdatos[causaACTV]);

  switch (causaACTV)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    this->inicio();
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    if (tv2seg(RTC_iniciado) == 0)
    {
      this->inicio();
    }
 
    this->actualizar("Sistema Activado");
   
    if ( !this->importar() )
    {
      RTC_reinicio = RTC_actual;
      timersub(&RTC_reinicio, &RTC_despertar, &RTC_error);
      settimeofday(&RTC_despertar, NULL);

      this->actualizar("Fecha INTERNA", "\t");
      Serial.printf("Error %f seg.\r\n", tv2seg_f(RTC_error));
    }
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

  RTC_despertar.tv_sec = SLEEP_INTERVALO + this->T_UTC - this->T_UTC % SLEEP_INTERVALO;
  RTC_despertar.tv_usec = 0;

  if (RTC_despertar.tv_sec < SLEEP_INTERVALO)
    RTC_despertar.tv_sec += SLEEP_INTERVALO;

  if (RTC_despertar.tv_sec % 3600 == 0)
  {
    RTC_iniciado.tv_sec = 0;
    RTC_iniciado.tv_usec = 0;
  }

  struct timeval tv;
  timersub(&RTC_despertar, &RTC_actual, &tv);

  float tiempo = tv2microseg(tv);
  float error = tiempo * 15 / 3600; //tv2microseg(RTC_error);

  // Serial.printf( "Suspendido durante %.3f segundos + %.3f de error, Actual: UTC%lu, Reinicio: UTC%lu\r\n",
  //             tiempo / 1000000, error / 1000000, tv2seg(RTC_actual), tv2seg(RTC_despertar) );
  Serial.printf("Suspendido durante %.3f segundos + %.3f de error, Actual: UTC%lu, Reinicio: UTC%lu\r\n",
                tv2seg_f(tv), error / 1000000, tv2seg(RTC_actual), tv2seg(RTC_despertar));

  Serial.println("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");

  if( this->dormir == nullptr)
    esp_deep_sleep(round(tiempo + error));
  else
    this->dormir(round(tiempo + error) );
}

fechaTOR FT;