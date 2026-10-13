/**
 * PC-Simulator der Bedieneinheit.
 *
 * Steuerung im Fenster:
 *   Maus           = Touch
 *   Mausrad        = Drehregler drehen
 *   Mittlere Taste = Drehregler druecken
 *   Tab            = naechstes Element
 *   Pfeil links/rechts = Wert aendern (Slider)
 *   Enter          = OK / druecken
 *
 * Sensorwerte und Hub werden hier nur simuliert.
 */
#define SDL_MAIN_HANDLED /* Windows: eigenes main() verwenden */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "lvgl.h"
#include "app/ui.h"
#include "app/hal.h"

static model_t model = {
    .temperature_c = 21.5f,
    .humidity_pct = 45.0f,
    .co2_ppm = 650,
    .window_open = false,
    .hub_connected = true,
    .light_on = false,
    .light_level = 50,
};

/* ---- HAL fuer den PC ---- */

void hal_send_light(bool on, int level)
{
    /* Am PC tun wir so, als haette der Hub den Befehl sofort umgesetzt. */
    model.light_on = on;
    model.light_level = level;
    printf("[Hub] Licht %s, Helligkeit %d %%\n", on ? "an" : "aus", level);
}

void hal_set_backlight(int percent)
{
    printf("[Display] Hintergrundbeleuchtung %d %%\n", percent);
}

/* Fake-Sensoren: langsam schwankende Werte, damit man etwas sieht. */
static void fake_sensors_cb(lv_timer_t * t)
{
    (void)t;
    float s = lv_tick_get() / 1000.0f;
    model.temperature_c = 21.5f + 1.5f * sinf(s / 20.0f);
    model.humidity_pct = 45.0f + 8.0f * sinf(s / 33.0f);
    model.co2_ppm = 900 + (int)(700.0f * sinf(s / 15.0f));
    model.window_open = ((int)s / 30) % 2 == 1;
    ui_update(&model);
}

int main(void)
{
    lv_init();

    lv_display_t * disp = lv_sdl_window_create(800, 480);
    lv_display_set_default(disp);

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);

    lv_sdl_mouse_create();
    lv_indev_t * wheel = lv_sdl_mousewheel_create();
    lv_indev_set_group(wheel, group);
    lv_indev_t * keyboard = lv_sdl_keyboard_create();
    lv_indev_set_group(keyboard, group);

    ui_init();
    ui_update(&model);
    lv_timer_create(fake_sensors_cb, 2000, NULL);

    while(1) {
        uint32_t idle_ms = lv_timer_handler();
        if(idle_ms > 10) idle_ms = 10;
        usleep(idle_ms * 1000);
    }
    return 0;
}
