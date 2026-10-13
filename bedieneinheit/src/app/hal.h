/**
 * Hardware-Schicht ("HAL"): diese Funktionen gibt es zweimal,
 * einmal in src/sim/ (PC) und einmal in src/esp32/ (CrowPanel).
 * Die Oberflaeche in src/app/ ruft nur diese Funktionen auf und weiss
 * nicht, ob sie am PC oder am Panel laeuft.
 */
#ifndef HAL_H
#define HAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Der Benutzer hat das Licht geschaltet oder gedimmt -> an den Hub schicken. */
void hal_send_light(bool on, int level);

/* Display-Hintergrundbeleuchtung 10..100 %. */
void hal_set_backlight(int percent);

#ifdef __cplusplus
}
#endif

#endif
