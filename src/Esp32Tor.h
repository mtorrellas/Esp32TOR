
#ifndef __ESP32TOR__
#define __ESP32TOR__

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <lwip/apps/sntp.h>
#include <Preferences.h>

#include "configTOR.h"

#define tv2microseg(tv) ( (tv).tv_sec * 1000000 + (tv).tv_usec )
#define tv2miliseg(tv) ( (tv).tv_sec * 1000 + round(  (float)(tv).tv_usec / 1000 ) )
#define tv2seg(tv) (time_t)(	(tv).tv_sec + round(  (float)(tv).tv_usec / 1000000 )  )//((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define tv2seg_f(tv) (float)( tv.tv_sec + (float)tv.tv_usec / 1000000 )

int _EXFUN(putenv, (char *__string));
int _EXFUN(setenv, (const char *__string, const char *__value, int __overwrite));
int _EXFUN(unsetenv, (const char *__string));
void _EXFUN(tzset,	(_VOID));

bool temporizador(uint16_t ms);
bool conectar_WiFi();
void SNTP(timeval &tv);
bool cambioEstado(uint alto, uint bajo);
struct _fTOR
{
  uint8_t segundo;
  uint8_t minuto;
  uint8_t hora;
  uint8_t hora12h;
  uint8_t diaSemana;
  uint8_t dia;
  uint8_t mes;
  uint16_t anio;
  time_t UTC;
  const char *cDia;
  const char *cMes;
  const char *cFecha;
  const char *cFechaCorta;
  const char *cFechaLarga;
  const char *cHora;
};

/* * * * * * * * * * * * * * * * * * * * * * * * 
 *                 Clase: fechaTor
 * Autor: Miguel A. Torrellas Delgado
 * Archivo: fechator.h
 * fechaTor:  Agosto 2021
 * Versión: 1.0.0
 * 
 * Descripción:
 * ========================
 * Controla fechas y funciones WiFi, de Deep Sleep, etc.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * */
class fechaTOR
{
private:
  void (*RTC_externo)()=nullptr ;
  void (*LED)( bool )=nullptr;
  void (*salida)()=nullptr;
  void (*entrada)()=nullptr;
  void (*dormir)(uint64_t);
  uint LED_On = 500;
  uint LED_Off = 500;
  bool LED_estado = true;
public:
  const char *NombreMes[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
  const char *NombreDia[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};

  const char *CET = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";

  _fTOR intercambio;
  uint8_t  T_segundo;
  uint8_t  T_minuto;
  uint8_t  T_hora;
  uint8_t  T_hora12h;
  uint8_t  T_diaSemana;
  uint8_t  T_dia;
  uint8_t  T_mes;
  uint16_t T_anio;
  time_t   T_UTC;
  const char *T_cDia;
  const char *T_cMes;
  char T_cFecha[15];
  char T_cFechaCorta[50];
  char T_cFechaLarga[100];
  char T_cHora[10];

/** * * * * * * * * * 
 * METODOS clase fechaTOR
 **/
private:
  void actualizar(const char * msg = "Fecha", const char * FL = "\r\n");
  void inicio();
  void suspender();
  void activar();
  void luzLED();
  bool importar();
  void exportar();
public:
  fechaTOR(/* args */);
  ~fechaTOR();
  void setLED(uint, uint);
  void begin( void(*led)(bool) = nullptr, void(*e)() = nullptr, void(*s)() = nullptr, void (*d)(uint64_t) = nullptr);
  bool update(int16_t s = 20);
};

extern esp_sleep_wakeup_cause_t causaACTV;
extern fechaTOR FT;
#define ft FT

#endif