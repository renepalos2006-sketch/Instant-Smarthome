/**
 * LVGL-Konfiguration fuer Simulator UND CrowPanel.
 * Alles, was hier nicht steht, nimmt LVGL mit Standardwerten an
 * (siehe lv_conf_template.h in der LVGL-Bibliothek).
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16

/* Speicher: die normale malloc()-Funktion verwenden.
 * Am ESP32 landen grosse Bloecke damit automatisch im 8 MB PSRAM. */
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#define LV_DEF_REFR_PERIOD 20 /* ms, ca. 50 FPS */

/* Schriften: gross genug fuer gute Lesbarkeit.
 * Achtung: die eingebauten Montserrat-Schriften haben keine Umlaute (ä ö ü ß).
 * Dafuer spaeter eine eigene Schrift mit dem LVGL Font Converter erzeugen. */
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_DEFAULT &lv_font_montserrat_20

#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 1

#ifdef SIMULATOR
    /* Fenster am PC ueber SDL2 */
    #define LV_USE_SDL 1
    #define LV_SDL_INCLUDE_PATH <SDL2/SDL.h>
    #define LV_SDL_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
    #define LV_SDL_BUF_COUNT 1
    #define LV_SDL_FULLSCREEN 0
    #define LV_SDL_DIRECT_EXIT 1
#endif

#endif /* LV_CONF_H */
