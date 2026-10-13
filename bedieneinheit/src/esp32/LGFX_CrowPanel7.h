/**
 * LovyanGFX-Konfiguration fuer das Elecrow CrowPanel 7.0" (800 x 480, ESP32-S3).
 * Pinbelegung und Timings aus dem offiziellen Elecrow-Beispiel.
 */
#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

#define CROWPANEL_I2C_SDA 19
#define CROWPANEL_I2C_SCL 20
#define CROWPANEL_BACKLIGHT 2

class LGFX : public lgfx::LGFX_Device {
public:
    lgfx::Bus_RGB _bus;
    lgfx::Panel_RGB _panel;
    lgfx::Light_PWM _light;
    lgfx::Touch_GT911 _touch;

    LGFX(void)
    {
        {
            auto cfg = _panel.config();
            cfg.memory_width = 800;
            cfg.memory_height = 480;
            cfg.panel_width = 800;
            cfg.panel_height = 480;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            _panel.config(cfg);
        }
        {
            auto cfg = _panel.config_detail();
            cfg.use_psram = 1;
            _panel.config_detail(cfg);
        }
        {
            auto cfg = _bus.config();
            cfg.panel = &_panel;
            // Blau
            cfg.pin_d0 = GPIO_NUM_15;
            cfg.pin_d1 = GPIO_NUM_7;
            cfg.pin_d2 = GPIO_NUM_6;
            cfg.pin_d3 = GPIO_NUM_5;
            cfg.pin_d4 = GPIO_NUM_4;
            // Gruen
            cfg.pin_d5 = GPIO_NUM_9;
            cfg.pin_d6 = GPIO_NUM_46;
            cfg.pin_d7 = GPIO_NUM_3;
            cfg.pin_d8 = GPIO_NUM_8;
            cfg.pin_d9 = GPIO_NUM_16;
            cfg.pin_d10 = GPIO_NUM_1;
            // Rot
            cfg.pin_d11 = GPIO_NUM_14;
            cfg.pin_d12 = GPIO_NUM_21;
            cfg.pin_d13 = GPIO_NUM_47;
            cfg.pin_d14 = GPIO_NUM_48;
            cfg.pin_d15 = GPIO_NUM_45;

            cfg.pin_henable = GPIO_NUM_41;
            cfg.pin_vsync = GPIO_NUM_40;
            cfg.pin_hsync = GPIO_NUM_39;
            cfg.pin_pclk = GPIO_NUM_0;
            cfg.freq_write = 15000000;

            cfg.hsync_polarity = 0;
            cfg.hsync_front_porch = 40;
            cfg.hsync_pulse_width = 48;
            cfg.hsync_back_porch = 40;
            cfg.vsync_polarity = 0;
            cfg.vsync_front_porch = 1;
            cfg.vsync_pulse_width = 31;
            cfg.vsync_back_porch = 13;
            cfg.pclk_active_neg = 1;
            cfg.de_idle_high = 0;
            cfg.pclk_idle_high = 0;
            _bus.config(cfg);
        }
        _panel.setBus(&_bus);

        {
            auto cfg = _light.config();
            cfg.pin_bl = CROWPANEL_BACKLIGHT;
            _light.config(cfg);
        }
        _panel.light(&_light);

        {
            auto cfg = _touch.config();
            cfg.x_min = 0;
            cfg.x_max = 799;
            cfg.y_min = 0;
            cfg.y_max = 479;
            cfg.pin_int = -1;
            cfg.pin_rst = -1;
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            cfg.i2c_port = I2C_NUM_1;
            cfg.pin_sda = CROWPANEL_I2C_SDA;
            cfg.pin_scl = CROWPANEL_I2C_SCL;
            cfg.freq = 400000;
            cfg.i2c_addr = 0x14; // LovyanGFX probiert automatisch auch 0x5D
            _touch.config(cfg);
            _panel.setTouch(&_touch);
        }

        setPanel(&_panel);
    }
};
