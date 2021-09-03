# Librería **"Preferences.h"**
## ***Quitar un espacio de nombres***

En la implementación de Arduino de ***"Preferences"***,  no hay ningún método para eliminar completamente un espacio de nombres. Como resultado, en el transcurso de varios proyectos, la partición de preferencias de almacenamiento no volátil (nvs) ESP32 puede estar llena. Para borrar y volver a formatear completamente la memoria NVS utilizada por Preferencias, cree un boceto que contenga:
~~~
#include <nvs_flash.h>

void setup() {
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init(); // initialize the NVS partition.
  while(true);
}

void loop() {}
~~~~
Debe descargar un nuevo boceto a su placa inmediatamente después de ejecutar lo anterior, o reformateará la partición NVS cada vez que se encienda.