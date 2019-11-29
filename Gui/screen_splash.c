//screen_splash.c

#include "gui.h"
#include "config.h"
#include "version.h"
#include "window_logo.h"

#include "stm32f4xx_hal.h"

#ifdef _EXTUI

#include "marlin_client.h"
extern screen_t* pscreen_home;

#else // MARLIN LCD UI

#ifdef LCDSIM
extern screen_t* pscreen_marlin;
#else
extern screen_t* pscreen_test;
#endif

#endif

#pragma pack(push)
#pragma pack(1)

typedef struct
{
    window_frame_t frame;
    window_logo_t logo_prusa_mini;
    window_text_t text_progress;
    window_progress_t progress;
    window_text_t text_version;
    window_icon_t icon_logo_buddy;
    window_icon_t icon_logo_marlin;

    window_icon_t icon_debug;

    uint32_t last_timer;
} screen_splash_data_t;

#pragma pack(pop)

#define _psd ((screen_splash_data_t*)screen->pdata)

void screen_splash_timer(screen_t* screen, uint32_t mseconds);

void screen_splash_init(screen_t* screen)
{
    int16_t id;
    int16_t id0;

    id0 = window_create_ptr(WINDOW_CLS_FRAME, -1, rect_ui16(0, 0, 0, 0),
        &(_psd->frame));

    id = window_create_ptr(WINDOW_CLS_LOGO, id0, rect_ui16(0, 91, 240, 62),
        &(_psd->logo_prusa_mini));

    id = window_create_ptr(WINDOW_CLS_TEXT, id0, rect_ui16(10, 171, 220, 20),
        &(_psd->text_progress));
    _psd->text_progress.font = resource_font(IDR_FNT_NORMAL);
    window_set_alignment(id, ALIGN_CENTER_BOTTOM);
    window_set_text(id, "Loading ...");

    id = window_create_ptr(WINDOW_CLS_PROGRESS, id0, rect_ui16(10, 200, 220, 15),
        &(_psd->progress));
    _psd->progress.color_back = COLOR_GRAY;
    _psd->progress.color_progress = COLOR_ORANGE;
    _psd->progress.font = resource_font(IDR_FNT_BIG);
    _psd->progress.height_progress = 15;

    id = window_create_ptr(WINDOW_CLS_ICON, id0, rect_ui16(80, 240, 80, 80),
        &(_psd->icon_logo_marlin));
    window_set_icon_id(id, IDR_PNG_splash_logo_marlin);

    id = window_create_ptr(WINDOW_CLS_TEXT, id0, rect_ui16(00, 295, 240, 22),
        &(_psd->text_version));
    window_set_alignment(id, ALIGN_CENTER);
    window_set_text(id, version_version);
}

void screen_splash_done(screen_t* screen)
{
    window_destroy(_psd->frame.win.id);
}

void screen_splash_draw(screen_t* screen)
{
}

int screen_splash_event(screen_t* screen, window_t* window, uint8_t event, void* param)
{
    screen_splash_timer(screen, HAL_GetTick());

#ifdef _EXTUI
    if (marlin_event(MARLIN_EVT_Startup)) {
        screen_close();
        screen_open(pscreen_home->id);
#else
    if (HAL_GetTick() > 3000) {
        screen_close();
#ifdef LCDSIM
        screen_open(pscreen_marlin->id);
#else
        screen_open(pscreen_test->id);
#endif
#endif
        return 1;
    }
    return 0;
}

void screen_splash_timer(screen_t* screen, uint32_t mseconds)
{
    float percent = mseconds / 3000.0 * 100;
    window_set_value(_psd->progress.win.id, (percent < 95) ? percent : 95);
}

screen_t screen_splash = {
    0,
    0,
    screen_splash_init,
    screen_splash_done,
    screen_splash_draw,
    screen_splash_event,
    sizeof(screen_splash_data_t), //data_size
    0, //pdata
};

const screen_t* pscreen_splash = &screen_splash;
