# MANCO-Gen1

Build personal de firmware para lightgun, basado en el port de OpenFIRE para ESP32, corriendo sobre ESP32-S3 (MANCO Gen1).

## Créditos

Este proyecto no arranca de cero: es un derivado directo del trabajo de la comunidad de lightguns DIY de código abierto.

* [**alessandro-satanassi/OpenFIRE-Firmware-ESP32**](https://github.com/alessandro-satanassi/OpenFIRE-Firmware-ESP32) — el port a ESP32-S3 sobre el que está construido este repositorio (v6.2.1), hecho por Alessandro Satanassi.
* A su vez basado en [**OpenFIRE-Firmware**](https://github.com/TeamOpenFIRE/OpenFIRE-Firmware) de TeamOpenFIRE (versión original, para RP2040).
* OpenFIRE es la continuación de **GUN4ALL**, desarrollado por **SeongGino**.
* Que a su vez se basa en el proyecto **SAMCO** original y su versión mejorada **SAMCO Enhanced**, de **Prow7**.

## Licencia

Este proyecto se distribuye bajo **LGPL-2.1**, la misma licencia que el firmware base. Ver el archivo [`LICENSE`](./LICENSE) para el texto completo.

* Podés usar, estudiar y modificar el código.
* Si distribuís una versión modificada (código o binario compilado), el código fuente de esa versión tiene que estar disponible bajo la misma licencia.
* Se debe mantener el aviso de copyright y la licencia original.

## Sobre este build

Basado en OpenFIRE-Firmware-ESP32 v6.2.1 (alessandro-satanassi), para ESP32-S3-WROOM1-DevKitC-1 (MANCO Gen1).

Cambios respecto al original:

* Los accesorios inalámbricos (pedal por ESP-NOW) siguen funcionando aunque la lightgun esté conectada por cable USB a la PC — el firmware original apaga el radio wireless por completo al detectar el cable, esto lo mantiene activo en paralelo solo para accesorios.
* Fix del bug de auto-recoil por hold en ESP32-S3: la lectura del estado del solenoide vía `digitalRead()` sobre un pin configurado como `OUTPUT` no es confiable en esta arquitectura (a diferencia de RP2040, donde sí lo es). Se agregó una variable de estado dedicada (`solenoidEngaged`) y un método `SolenoidWrite()` para reemplazar esas lecturas directas del pin.

## Autor de este fork

Juguetoide

