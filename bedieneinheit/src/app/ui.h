#ifndef UI_H
#define UI_H

#include "lvgl.h"
#include "app/model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Baut alle Screens auf. Vorher muss eine lv_group als Standard-Gruppe
 * gesetzt sein (lv_group_set_default), damit Tasten/Drehregler funktionieren. */
void ui_init(void);

/* Neue Messwerte bzw. Hub-Zustand anzeigen. */
void ui_update(const model_t * m);

#ifdef __cplusplus
}
#endif

#endif
