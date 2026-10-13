/**
 * Firmware der Bedieneinheit fuer das CrowPanel 7.0".
 *
 * Gleiche Oberflaeche wie im Simulator (src/app/), nur die Hardware-Schicht ist anders:
 *   - Display und Touch ueber LovyanGFX
 *   - Tasten/Drehregler: solange die Platine noch nicht da ist, ueber den
 *     seriellen Monitor (115200 Baud):  w = vorheriges, s = naechstes,
 *     a/d = Wert verringern/erhoehen, e = OK, q = zurueck
 *   - Sensoren und Hub: vorerst Fake-Werte (TODO)
 */
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include "LGFX_CrowPanel7.h"
#include "app/ui.h"
#include "app/hal.h"

static LGFX lcd;
static model_t model = {21.5f, 45.0f, 650, false, false, false, 50};

/* ---------- HAL fuer das Panel ---------- */

void hal_send_light(bool on, int level)
{
    // TODO: per ESP-NOW an die Lampe (Hub) schicken. Bis dahin: lokal uebernehmen.
    model.light_on = on;
    model.light_level = level;
    Serial.printf("[Hub] Licht %s, Helligkeit %d %%\n", on ? "an" : "aus", level);
}

void hal_set_backlight(int percent)
{
    lcd.setBrightness(map(percent, 0, 100, 0, 255));
}

/* ---------- Hardware-Version 3.0: Touch-Reset ueber den PCA9557 ---------- */

static void pca9557_write(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(0x18);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

static void touch_reset_v3(void)
{
    Wire.begin(CROWPANEL_I2C_SDA, CROWPANEL_I2C_SCL);
    Wire.beginTransmission(0x18);
    bool has_pca9557 = Wire.endTransmission() == 0;
    if(has_pca9557) {
        // Ablauf wie im Elecrow-Beispiel: IO0 (Touch-Reset) und IO1 (Touch-INT) low,
        // dann Reset loslassen, danach INT wieder als Eingang.
        pca9557_write(0x03, 0x00);  // alle Pins Ausgang
        pca9557_write(0x01, 0x00);  // IO0 = IO1 = low
        delay(20);
        pca9557_write(0x01, 0x01);  // IO0 = high
        delay(100);
        pca9557_write(0x03, 0x02);  // IO1 = Eingang
        Serial.println("Hardware V3.0 erkannt (PCA9557), Touch zurueckgesetzt");
    }
    Wire.end();
}

/* ---------- LVGL-Anbindung ---------- */

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    lcd.pushImage(area->x1, area->y1, w, h, (lgfx::rgb565_t *)px);
    lv_display_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    uint16_t x, y;
    if(lcd.getTouch(&x, &y)) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* Platzhalter fuer die echten Tasten: Zeichen aus dem seriellen Monitor. */
static void keys_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;
    static bool pressed = false;

    if(pressed) {  // jede Taste kurz "druecken" und im naechsten Durchlauf "loslassen"
        pressed = false;
        data->key = last_key;
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    if(Serial.available()) {
        switch(Serial.read()) {
            case 'w': last_key = LV_KEY_PREV; break;
            case 's': last_key = LV_KEY_NEXT; break;
            case 'a': last_key = LV_KEY_LEFT; break;
            case 'd': last_key = LV_KEY_RIGHT; break;
            case 'e': last_key = LV_KEY_ENTER; break;
            case 'q': last_key = LV_KEY_ESC; break;
            default: data->state = LV_INDEV_STATE_RELEASED; return;
        }
        pressed = true;
        data->key = last_key;
        data->state = LV_INDEV_STATE_PRESSED;
        return;
    }
    data->key = last_key;
    data->state = LV_INDEV_STATE_RELEASED;
}

static uint32_t tick_cb(void)
{
    return millis();
}

/* Fake-Sensoren, bis die Platine mit SHT4x/SCD4x da ist. */
static void fake_sensors_cb(lv_timer_t * t)
{
    float s = millis() / 1000.0f;
    model.temperature_c = 21.5f + 1.5f * sinf(s / 20.0f);
    model.humidity_pct = 45.0f + 8.0f * sinf(s / 33.0f);
    model.co2_ppm = 900 + (int)(700.0f * sinf(s / 15.0f));
    ui_update(&model);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Bedieneinheit startet");

    touch_reset_v3();
    lcd.begin();
    lcd.setBrightness(255);
    lcd.fillScreen(TFT_BLACK);

    lv_init();
    lv_tick_set_cb(tick_cb);

    lv_display_t * disp = lv_display_create(800, 480);
    const size_t buf_px = 800 * 48;  // 48 Zeilen pro Teil-Neuzeichnung
    void * buf1 = heap_caps_malloc(buf_px * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    void * buf2 = heap_caps_malloc(buf_px * 2, MALLOC_CAP_SPIRAM);
    lv_display_set_buffers(disp, buf1, buf2, buf_px * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flush_cb);

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);

    lv_indev_t * touch = lv_indev_create();
    lv_indev_set_type(touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch, touch_read_cb);

    lv_indev_t * keys = lv_indev_create();
    lv_indev_set_type(keys, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(keys, keys_read_cb);
    lv_indev_set_group(keys, group);

    ui_init();
    ui_update(&model);
    lv_timer_create(fake_sensors_cb, 2000, NULL);

    Serial.println("Bereit. Tasten im seriellen Monitor: w/s = Fokus, a/d = Wert, e = OK");
}

void loop()
{
    uint32_t idle_ms = lv_timer_handler();
    delay(idle_ms < 5 ? idle_ms : 5);
}
