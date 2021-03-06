/*
 * window_dlg_preheat.c
 *
 *  Created on: 2019-11-18
 *      Author: Vana Radek
 */

#include "window_dlg_preheat.h"
#include "display_helper.h"
#include "gui.h"
#include "dbg.h"
#include "stm32f4xx_hal.h"
#include "filament.h"
#include "marlin_client.h"
#include "resource.h"
#include "stdlib.h"
//"inherit" those functions, to inherit frame behavior
extern void window_frame_done(window_frame_t* window);
extern void window_frame_draw(window_frame_t* window);
extern void window_frame_event(window_frame_t* window, uint8_t event, void* param);

int16_t WINDOW_CLS_DLG_PREHEAT = 0;

extern window_t* window_1; //current popup window

#define _PREHEAT_FILAMENT_CNT (FILAMENTS_END - FILAMENT_PLA)

//no return option
void window_list_filament_item_forced_cb(window_list_t* pwindow_list, uint16_t index,
    const char** pptext, uint16_t* pid_icon)
{
    if (index <= pwindow_list->count) {
        *pptext = filaments[index + FILAMENT_PLA].long_name;
    } else
        *pptext = "Index ERROR";

    *pid_icon = 0;
}

void window_list_filament_item_cb(window_list_t* pwindow_list, uint16_t index,
    const char** pptext, uint16_t* pid_icon)
{
    if (index == 0) {
        *pptext = "Return";
        *pid_icon = IDR_PNG_filescreen_icon_up_folder;
    } else {
        window_list_filament_item_forced_cb(pwindow_list, index - 1, pptext, pid_icon);
    }
}

static void _set_filament(FILAMENT_t index)
{
    marlin_gcode_printf("M104 S%d", (int)filaments[index].nozzle);
    marlin_gcode_printf("M140 S%d", (int)filaments[index].heatbed);
    set_filament(index);
}

void window_dlg_preheat_click_forced_cb(window_dlg_preheat_t* window)
{
    FILAMENT_t index = window->list.index + FILAMENT_PLA;
    _set_filament(index);
}

void window_dlg_preheat_click_cb(window_dlg_preheat_t* window)
{
    if (window->list.index > 0) {
        FILAMENT_t index = window->list.index + FILAMENT_PLA - 1;
        _set_filament(index);
    }
}

void window_dlg_preheat_init(window_dlg_preheat_t* window)
{
    //inherit from frame
    window_class_frame.cls.init(window);
    window->win.flg |= WINDOW_FLG_ENABLED | WINDOW_FLG_INVALID;
    window->color_back = gui_defaults.color_back;
    window->color_text = gui_defaults.color_text;
    window->font = gui_defaults.font;
    window->font_title = gui_defaults.font_big;
    window->padding = gui_defaults.padding;

    int16_t id;
    rect_ui16_t rect = gui_defaults.msg_box_sz;
    if (window->caption) {
        rect.h = window->font_title->h + 2;
        id = window_create_ptr(WINDOW_CLS_TEXT, window->win.id, rect, &(window->text));
        window_set_text(id, window->caption);
        rect = gui_defaults.msg_box_sz;
        rect.y += window->font_title->h + 4;
        rect.h -= window->font_title->h + 4;
    }

    id = window_create_ptr(WINDOW_CLS_LIST, window->win.id, rect, &(window->list));
    window->list.padding = padding_ui8(20, 6, 2, 6);
    window->list.icon_rect = rect_ui16(0, 0, 16, 30);

    //window_set_item_count(id, window->filaments_count);
    window_set_item_index(id, 0);
    window_set_item_callback(id, window->filament_items);
}

void window_dlg_preheat_event(window_dlg_preheat_t* window, uint8_t event, void* param)
{
    //todo, fixme, error i will not get WINDOW_EVENT_BTN_DN event here first time when it is clicked
    //but list reacts to it
    switch (event) {
    case WINDOW_EVENT_BTN_DN:
    case WINDOW_EVENT_BTN_UP:
    case WINDOW_EVENT_CLICK:
        if (window->timer != -1) {
            window->timer = -1; //close
            window->on_click(window);
        }
        return;
    default:
        window_frame_event((void*)window, event, param);
    }
}

const window_class_dlg_preheat_t window_class_dlg_preheat = {
    {
        //call frame methods
        WINDOW_CLS_USER,
        sizeof(window_dlg_preheat_t),
        (window_init_t*)window_dlg_preheat_init, //must call window_frame_init inside
        (window_done_t*)window_frame_done,
        (window_draw_t*)window_frame_draw,
        (window_event_t*)window_dlg_preheat_event, //must call window_frame_event
    },
};

int gui_dlg_preheat(const char* caption)
{
    return gui_dlg_list(
        caption,
        window_list_filament_item_cb,
        window_dlg_preheat_click_cb,
        _PREHEAT_FILAMENT_CNT + 1, //+1 back option
        30000);
}

//no return option
int gui_dlg_preheat_forced(const char* caption)
{
    return gui_dlg_list(
        caption,
        window_list_filament_item_forced_cb,
        window_dlg_preheat_click_forced_cb,
        _PREHEAT_FILAMENT_CNT,
        -1 //do not leave
    );
}

//returns index or -1 on timeout
//todo make this independet on preheat, in separate file
//todo caption is not showing
int gui_dlg_list(const char* caption, window_list_item_t* filament_items,
    dlg_on_click_cb* on_click, size_t count, int32_t ttl)
{
    window_dlg_preheat_t dlg;
    dlg.caption = caption;
    dlg.filament_items = filament_items;
    dlg.on_click = on_click;
    //dlg.filaments_count = count;

    //parent 0 would be first screen
    //here must be -1
    int16_t id_capture = window_capture();
    int16_t id = window_create_ptr(WINDOW_CLS_DLG_PREHEAT, -1, gui_defaults.msg_box_sz, &dlg);

    window_set_item_count(dlg.list.win.id, count);

    window_1 = (window_t*)&dlg;
    window_set_capture(dlg.list.win.id); //get inside list

    gui_reset_jogwheel();
    gui_invalidate();

    //window_disable(id);

    dlg.timer = HAL_GetTick();

    //ttl retyped to uint - so "-1" == for ever (or very long)
    while ((dlg.timer != -1) && ((uint32_t)(HAL_GetTick() - dlg.timer) < (uint32_t)ttl)) {
        gui_loop();
    }

    int ret;
    if (dlg.timer != -1) {
        ret = -1;
    } else {
        ret = dlg.list.index;
    }

    window_destroy(id); //msg box does not call this, should I
    window_invalidate(0);
    window_set_capture(id_capture);
    return ret;
}
