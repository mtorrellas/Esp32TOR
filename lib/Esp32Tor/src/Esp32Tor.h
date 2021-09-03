
#ifndef __ESP32TOR__
#define __ESP32TOR__

#define _FT_DEBUG

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
//#include <cstdlib>
#include <lwip/apps/sntp.h>
#include <Preferences.h>

#define SLEEP_INTERVALO 300  //900

#define tv2microseg(tv) ( (tv).tv_sec * 1000000 + (tv).tv_usec )
#define tv2miliseg(tv) ( (tv).tv_sec * 1000 + round(  (float)(tv).tv_usec / 1000 ) )
#define tv2seg(tv) (time_t)(	(tv).tv_sec + round(  (float)(tv).tv_usec / 1000000 )  )//((tvp)->tv_sec = (tvp)->tv_usec = 0)

int _EXFUN(putenv, (char *__string));
int _EXFUN(setenv, (const char *__string, const char *__value, int __overwrite));
int _EXFUN(unsetenv, (const char *__string));
void _EXFUN(tzset,	(_VOID));

bool temporizador(uint16_t ms);
bool conectar_WiFi();
void SNTP(timeval &tv);

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
  void (*RTC_externo)();
  void actualizar(const char * msg = "Fecha");
  void suspender();
public:
  const char *NombreMes[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
  const char *NombreDia[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};

  const char *CET = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";
  fechaTOR(/* args */);
  ~fechaTOR();


  uint8_t  segundo;
  uint8_t  minuto;
  uint8_t  hora;
  uint8_t  hora12h;
  uint8_t  diaSemana;
  uint8_t  dia;
  uint8_t  mes;
  uint16_t anio;
  time_t   UTC;
  const char *cDia;
  const char *cMes;
  char cFecha[50];
  char cFechaCorta[50];
  char cFechaLarga[100];
  char cHora[10];

  //void begin();
  void begin(  bool (*)(uint16_t) = nullptr );
  void activar();
  bool update(uint16_t s = 20);
};

extern fechaTOR FT;
#define ft FT

#endif