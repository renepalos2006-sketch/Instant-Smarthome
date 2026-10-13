/**
 * Oberflaeche der Bedieneinheit (800 x 480).
 *
 * Aufbau:
 *   +---------------------------------------------------------+
 *   | Instant Smarthome                        Hub verbunden  |  Kopfzeile
 *   +-----------+---------------------------------------------+
 *   |  Start    |                                             |
 *   |  Licht    |        Inhalt der gewaehlten Seite          |
 *   |  System   |                                             |
 *   +-----------+---------------------------------------------+
 *
 * Bedienung: Touch, Drehregler (ENCODER) oder Tasten (KEYPAD).
 * Alle bedienbaren Elemente landen automatisch in der Standard-Gruppe,
 * dadurch springt der Fokus mit jedem Dreh-Schritt zum naechsten Element.
 * Elemente auf versteckten Seiten ueberspringt LVGL von selbst.
 */
#include "app/ui.h"
#include "app/hal.h"
#include <stdio.h>

#define NAV_WIDTH 200
#define HEADER_HEIGHT 64

#define COLOR_BG lv_color_hex(0xF2F4F7)
#define COLOR_CARD lv_color_hex(0xFFFFFF)
#define COLOR_TEXT lv_color_hex(0x1D2939)
#define COLOR_MUTED lv_color_hex(0x667085)
#define COLOR_ACCENT lv_color_hex(0x1570EF)
#define COLOR_GOOD lv_color_hex(0x12B76A)
#define COLOR_WARN lv_color_hex(0xF79009)
#define COLOR_BAD lv_color_hex(0xD92D20)

enum { PAGE_START, PAGE_LIGHT, PAGE_SYSTEM, PAGE_COUNT };

static lv_obj_t * pages[PAGE_COUNT];
static lv_obj_t * nav_buttons[PAGE_COUNT];

static lv_obj_t * hub_label;
static lv_obj_t * temp_value;
static lv_obj_t * hum_value;
static lv_obj_t * co2_value;
static lv_obj_t * window_value;
static lv_obj_t * light_switch;
static lv_obj_t * light_slider;
static lv_obj_t * light_value;
static lv_obj_t * backlight_value;

/* ---------- Hilfsfunktionen ---------- */

static void show_page(int page)
{
    for(int i = 0; i < PAGE_COUNT; i++) {
        if(i == page) {
            lv_obj_remove_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_state(nav_buttons[i], LV_STATE_CHECKED);
        }
        else {
            lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_state(nav_buttons[i], LV_STATE_CHECKED);
        }
    }
}

static lv_obj_t * create_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(page, 20, 0);
    lv_obj_set_style_pad_gap(page, 20, 0);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_remove_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    return page;
}

static lv_obj_t * create_card(lv_obj_t * parent, int width, int height)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_size(card, width, height);
    lv_obj_set_style_bg_color(card, COLOR_CARD, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_shadow_width(card, 12, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_10, 0);
    lv_obj_set_style_pad_all(card, 18, 0);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

/* Kachel mit Titel oben und grossem Messwert darunter (nur Anzeige, nicht fokussierbar). */
static lv_obj_t * create_value_tile(lv_obj_t * parent, const char * title)
{
    lv_obj_t * card = create_card(parent, 265, 170);

    lv_obj_t * t = lv_label_create(card);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, COLOR_MUTED, 0);
    lv_obj_align(t, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t * v = lv_label_create(card);
    lv_label_set_text(v, "--");
    lv_obj_set_style_text_font(v, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(v, COLOR_TEXT, 0);
    lv_obj_align(v, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    return v;
}

/* Fokus-Rahmen gut sichtbar machen: wichtig fuer Bedienung mit Drehregler. */
static void style_focus(lv_obj_t * obj)
{
    lv_obj_set_style_outline_width(obj, 4, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_color(obj, COLOR_ACCENT, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(obj, 4, LV_STATE_FOCUS_KEY);
}

/* ---------- Ereignisse ---------- */

static void nav_clicked_cb(lv_event_t * e)
{
    int page = (int)(intptr_t)lv_event_get_user_data(e);
    show_page(page);
}

static void light_changed_cb(lv_event_t * e)
{
    (void)e;
    bool on = lv_obj_has_state(light_switch, LV_STATE_CHECKED);
    int level = lv_slider_get_value(light_slider);
    lv_label_set_text_fmt(light_value, "%d %%", level);
    hal_send_light(on, level);
}

static void backlight_changed_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int percent = lv_slider_get_value(slider);
    lv_label_set_text_fmt(backlight_value, "%d %%", percent);
    hal_set_backlight(percent);
}

/* ---------- Seiten ---------- */

static void build_start_page(lv_obj_t * page)
{
    temp_value = create_value_tile(page, "Temperatur");
    hum_value = create_value_tile(page, "Luftfeuchte");
    co2_value = create_value_tile(page, "CO2");
    window_value = create_value_tile(page, "Fenster");
}

static void build_light_page(lv_obj_t * page)
{
    lv_obj_t * card = create_card(page, 550, 340);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_CHARGE "  Stehlampe");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    light_switch = lv_switch_create(card);
    lv_obj_set_size(light_switch, 120, 60);
    lv_obj_align(light_switch, LV_ALIGN_TOP_RIGHT, 0, -6);
    style_focus(light_switch);
    lv_obj_add_event_cb(light_switch, light_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * dim_label = lv_label_create(card);
    lv_label_set_text(dim_label, "Helligkeit");
    lv_obj_set_style_text_color(dim_label, COLOR_MUTED, 0);
    lv_obj_align(dim_label, LV_ALIGN_LEFT_MID, 0, 10);

    light_value = lv_label_create(card);
    lv_label_set_text(light_value, "50 %");
    lv_obj_set_style_text_font(light_value, &lv_font_montserrat_28, 0);
    lv_obj_align(light_value, LV_ALIGN_RIGHT_MID, 0, 10);

    light_slider = lv_slider_create(card);
    lv_obj_set_size(light_slider, 480, 30);
    lv_slider_set_range(light_slider, 0, 100);
    lv_slider_set_value(light_slider, 50, LV_ANIM_OFF);
    lv_obj_align(light_slider, LV_ALIGN_BOTTOM_MID, 0, -30);
    style_focus(light_slider);
    lv_obj_add_event_cb(light_slider, light_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

static void build_system_page(lv_obj_t * page)
{
    lv_obj_t * card = create_card(page, 550, 200);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, "Display-Helligkeit");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    backlight_value = lv_label_create(card);
    lv_label_set_text(backlight_value, "100 %");
    lv_obj_set_style_text_font(backlight_value, &lv_font_montserrat_28, 0);
    lv_obj_align(backlight_value, LV_ALIGN_TOP_RIGHT, 0, -4);

    lv_obj_t * slider = lv_slider_create(card);
    lv_obj_set_size(slider, 480, 30);
    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -20);
    style_focus(slider);
    lv_obj_add_event_cb(slider, backlight_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * info = lv_label_create(page);
    lv_label_set_text(info, "Instant Smarthome - Bedieneinheit v0.1");
    lv_obj_set_style_text_color(info, COLOR_MUTED, 0);
}

/* ---------- Aufbau ---------- */

void ui_init(void)
{
    lv_obj_t * scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_text_color(scr, COLOR_TEXT, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* Kopfzeile */
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, LV_PCT(100), HEADER_HEIGHT);
    lv_obj_set_style_bg_color(header, COLOR_ACCENT, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_hor(header, 24, 0);

    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "Instant Smarthome");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    hub_label = lv_label_create(header);
    lv_label_set_text(hub_label, "");
    lv_obj_set_style_text_color(hub_label, lv_color_white(), 0);
    lv_obj_align(hub_label, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Navigation links */
    lv_obj_t * nav = lv_obj_create(scr);
    lv_obj_remove_style_all(nav);
    lv_obj_set_size(nav, NAV_WIDTH, 480 - HEADER_HEIGHT);
    lv_obj_align(nav, LV_ALIGN_TOP_LEFT, 0, HEADER_HEIGHT);
    lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(nav, 16, 0);
    lv_obj_set_style_pad_gap(nav, 14, 0);

    static const char * nav_texts[PAGE_COUNT] = {
        LV_SYMBOL_HOME "  Start",
        LV_SYMBOL_CHARGE "  Licht",
        LV_SYMBOL_SETTINGS "  System",
    };
    for(int i = 0; i < PAGE_COUNT; i++) {
        lv_obj_t * btn = lv_button_create(nav);
        lv_obj_set_size(btn, LV_PCT(100), 80);
        lv_obj_set_style_bg_color(btn, COLOR_CARD, 0);
        lv_obj_set_style_text_color(btn, COLOR_TEXT, 0);
        lv_obj_set_style_bg_color(btn, COLOR_ACCENT, LV_STATE_CHECKED);
        lv_obj_set_style_text_color(btn, lv_color_white(), LV_STATE_CHECKED);
        lv_obj_set_style_radius(btn, 14, 0);
        style_focus(btn);
        lv_obj_add_event_cb(btn, nav_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text(label, nav_texts[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 4, 0);
        nav_buttons[i] = btn;
    }

    /* Inhaltsbereich rechts */
    lv_obj_t * content = lv_obj_create(scr);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, 800 - NAV_WIDTH, 480 - HEADER_HEIGHT);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, NAV_WIDTH, HEADER_HEIGHT);

    for(int i = 0; i < PAGE_COUNT; i++) pages[i] = create_page(content);
    build_start_page(pages[PAGE_START]);
    build_light_page(pages[PAGE_LIGHT]);
    build_system_page(pages[PAGE_SYSTEM]);

    show_page(PAGE_START);
}

void ui_update(const model_t * m)
{
    char buf[32];

    snprintf(buf, sizeof(buf), "%.1f \xC2\xB0" "C", (double)m->temperature_c);
    lv_label_set_text(temp_value, buf);

    snprintf(buf, sizeof(buf), "%.0f %%", (double)m->humidity_pct);
    lv_label_set_text(hum_value, buf);

    lv_label_set_text_fmt(co2_value, "%d ppm", m->co2_ppm);
    lv_color_t co2_color = m->co2_ppm < 1000 ? COLOR_GOOD : (m->co2_ppm < 1500 ? COLOR_WARN : COLOR_BAD);
    lv_obj_set_style_text_color(co2_value, co2_color, 0);

    lv_label_set_text(window_value, m->window_open ? "offen" : "zu");

    lv_label_set_text(hub_label, m->hub_connected ? LV_SYMBOL_WIFI "  Hub verbunden" : LV_SYMBOL_WARNING "  Kein Hub");

    /* Lampen-Zustand vom Hub uebernehmen, aber nicht waehrend der Benutzer gerade daran dreht. */
    if(!lv_obj_has_state(light_slider, LV_STATE_EDITED) && !lv_obj_has_state(light_slider, LV_STATE_PRESSED)) {
        if(m->light_on) lv_obj_add_state(light_switch, LV_STATE_CHECKED);
        else lv_obj_remove_state(light_switch, LV_STATE_CHECKED);
        lv_slider_set_value(light_slider, m->light_level, LV_ANIM_OFF);
        lv_label_set_text_fmt(light_value, "%d %%", m->light_level);
    }
}
