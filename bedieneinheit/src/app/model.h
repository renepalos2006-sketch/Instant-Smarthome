/**
 * Datenmodell der Bedieneinheit: alles, was die Oberflaeche anzeigt.
 * Wird am PC mit Fake-Werten und am Panel mit echten Sensor- und Hub-Daten gefuellt.
 */
#ifndef MODEL_H
#define MODEL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature_c;  /* Temperatur in °C */
    float humidity_pct;   /* relative Luftfeuchte in % */
    int co2_ppm;          /* CO2 in ppm */
    bool window_open;     /* Fensterkontakt (vom Hub) */
    bool hub_connected;   /* Verbindung zur Lampe */
    bool light_on;        /* Zustand der Lampe */
    int light_level;      /* Helligkeit 0..100 % */
} model_t;

#ifdef __cplusplus
}
#endif

#endif
