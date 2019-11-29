//guimain.c

#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "gui.h"
#include "config.h"
#include "marlin_client.h"

#include "window_logo.h"

#ifdef LCDSIM
#include "window_lcdsim.h"
#else //LCDSIM
#include "window_file_list.h"
#include "window_header.h"
#include "window_temp_graph.h"
#include "window_dlg_loadunload.h"
#include "window_dlg_wait.h"
#include "window_dlg_popup.h"
#include "window_dlg_preheat.h"
#include "screen_print_preview.h"
#endif //LCDSIM

extern screen_t* pscreen_splash;
extern screen_t* pscreen_watchdog;

#ifdef LCDSIM
extern screen_t* pscreen_marlin;
#else //LCDSIM
extern screen_t* pscreen_test;
extern screen_t* pscreen_test_gui;
extern screen_t* pscreen_test_term;
extern screen_t* pscreen_test_msgbox;
extern screen_t* pscreen_test_graph;
extern screen_t* pscreen_test_temperature;
extern screen_t* pscreen_home;
extern screen_t* pscreen_filebrowser;
extern screen_t* pscreen_printing;
extern screen_t* pscreen_menu_preheat;
extern screen_t* pscreen_menu_filament;
extern screen_t* pscreen_preheating;
extern screen_t* pscreen_menu_calibration;
extern screen_t* pscreen_menu_settings;
extern screen_t* pscreen_menu_temperature;
extern screen_t* pscreen_menu_move;
extern screen_t* pscreen_menu_info;
extern screen_t* pscreen_menu_tune;
extern screen_t* pscreen_menu_service;
extern screen_t* pscreen_sysinfo;
extern screen_t* pscreen_version_info;
extern screen_t* pscreen_test_disp_mem;
extern screen_t* pscreen_messages;
extern screen_t* pscreen_logging;
#ifdef PIDCALIBRATION
extern screen_t* pscreen_PID;
#endif //PIDCALIBRATION
extern screen_t* pscreen_mesh_bed_lv;
extern screen_t* pscreen_wizard;
#endif // LCDSIM

extern int HAL_IWDG_Reset;

extern SPI_HandleTypeDef hspi2;

#ifndef _DEBUG
extern IWDG_HandleTypeDef hiwdg; //watchdog handle
#endif //_DEBUG

int guimain_spi_test = 0;

#include "gpio.h"
#include "st7789v.h"
#include "jogwheel.h"
#include "hwio_a3ides.h"
#include "diag.h"
#include "sys.h"
#include "dbg.h"
#include "marlin_host.h"

const st7789v_config_t st7789v_cfg = {
    &hspi2, // spi handle pointer
    ST7789V_PIN_CS, // CS pin
    ST7789V_PIN_RS, // RS pin
    ST7789V_PIN_RST, // RST pin
    ST7789V_FLG_DMA, // flags (DMA, MISO)
    ST7789V_DEF_COLMOD, // interface pixel format (5-6-5, hi-color)
    ST7789V_DEF_MADCTL, // memory data access control (no mirror XY)
};

const jogwheel_config_t jogwheel_cfg = {
    JOGWHEEL_PIN_EN1, // encoder phase1
    JOGWHEEL_PIN_EN2, // encoder phase2
    JOGWHEEL_PIN_ENC, // button
    JOGWHEEL_DEF_FLG, // flags
};

marlin_vars_t* gui_marlin_vars = 0;
int8_t menu_timeout_enabled = 1; // Default: enabled

void update_firmware_screen(void);

void gui_run(void)
{
    if (diag_fastboot)
        return;

    st7789v_config = st7789v_cfg;
    jogwheel_config = jogwheel_cfg;
    gui_init();
    gui_defaults.font = resource_font(IDR_FNT_NORMAL);
    gui_defaults.font_big = resource_font(IDR_FNT_BIG);
    window_msgbox_id_icon[0] = 0; //IDR_PNG_icon_pepa;
    window_msgbox_id_icon[1] = IDR_PNG_header_icon_error;
    window_msgbox_id_icon[2] = IDR_PNG_header_icon_question;
    window_msgbox_id_icon[3] = IDR_PNG_header_icon_warning;
    window_msgbox_id_icon[4] = IDR_PNG_header_icon_info;

#if (0)
    if (!sys_fw_is_valid())
        update_firmware_screen();
#endif

    gui_marlin_vars = marlin_client_init();

    hwio_beeper_tone2(440.0, 100, 0.0125); //start beep

    screen_register(pscreen_splash);
    screen_register(pscreen_watchdog);

    WINDOW_CLS_LOGO = window_register_class((window_class_t*)&window_class_logo);
#ifdef LCDSIM
    WINDOW_CLS_LCDSIM = window_register_class((window_class_t*)&window_class_lcdsim);
    screen_register(pscreen_marlin);
#else //LCDSIM
    WINDOW_CLS_FILE_LIST = window_register_class((window_class_t*)&window_class_file_list);
    WINDOW_CLS_HEADER = window_register_class((window_class_t*)&window_class_header);
    WINDOW_CLS_TEMP_GRAPH = window_register_class((window_class_t*)&window_class_temp_graph);
    WINDOW_CLS_DLG_LOADUNLOAD = window_register_class((window_class_t*)&window_class_dlg_loadunload);
    WINDOW_CLS_DLG_WAIT = window_register_class((window_class_t*)&window_class_dlg_wait);
    WINDOW_CLS_DLG_POPUP = window_register_class((window_class_t*)&window_class_dlg_popup);
    WINDOW_CLS_DLG_PREHEAT = window_register_class((window_class_t*)&window_class_dlg_preheat);
    screen_register(pscreen_test);
    screen_register(pscreen_test_gui);
    screen_register(pscreen_test_term);
    screen_register(pscreen_test_msgbox);
    screen_register(pscreen_test_graph);
    screen_register(pscreen_test_temperature);
    screen_register(pscreen_home);
    screen_register(pscreen_filebrowser);
    screen_register(pscreen_printing);
    screen_register(pscreen_menu_preheat);
    screen_register(pscreen_menu_filament);
    screen_register(pscreen_preheating);
    screen_register(pscreen_menu_calibration);
    screen_register(pscreen_menu_settings);
    screen_register(pscreen_menu_temperature);
    screen_register(pscreen_menu_move);
    screen_register(pscreen_menu_info);
    screen_register(pscreen_menu_tune);
    screen_register(pscreen_menu_service);
    screen_register(pscreen_sysinfo);
    screen_register(pscreen_version_info);
    screen_register(pscreen_test_disp_mem);
    screen_register(pscreen_messages);
#ifdef PIDCALIBRATION
    screen_register(pscreen_PID);
#endif //PIDCALIBRATION
    screen_register(pscreen_mesh_bed_lv);
    screen_register(pscreen_wizard);
    screen_register(pscreen_print_preview);
    screen_register(pscreen_logging);
#endif // LCDSIM

#ifndef _DEBUG
    if (HAL_IWDG_Reset) {
        screen_stack_push(pscreen_splash->id);
        screen_open(pscreen_watchdog->id);
    } else
#endif // _DEBUG
        screen_open(pscreen_splash->id);

    //set loop callback (will be called every time inside gui_loop)
    gui_loop_cb = marlin_client_loop;
    int8_t gui_timeout_id;
    while (1) {
        float vol = 0.01F;
        //simple jogwheel acoustic feedback
        if ((jogwheel_changed & 1) && jogwheel_button_down) //button changed and pressed
            hwio_beeper_tone2(200.0, 50, (double)(vol * 0.125F)); //beep
        else if (jogwheel_changed & 2) // encoder changed
            hwio_beeper_tone2(50.0, 25, (double)(vol * 0.125F)); //short click
        // show warning dialog on safety timer expiration
        if (marlin_event_clr(MARLIN_EVT_SafetyTimerExpired)) {
            gui_msgbox("Heating disabled by safety timer.", MSGBOX_BTN_OK | MSGBOX_ICO_WARNING);
        }
        gui_loop();
#ifndef LCDSIM
        if (marlin_message_received()) {
            screen_t* curr = screen_get_curr();
            if (curr == pscreen_printing)
                gui_pop_up();
        }
        if (menu_timeout_enabled) {
            gui_timeout_id = gui_get_menu_timeout_id();
            if (gui_timer_expired(gui_timeout_id) == 1) {
                screen_t* curr = screen_get_curr();
                if (
                    curr != pscreen_menu_tune && curr != pscreen_wizard && curr != pscreen_print_preview) { //timeout screen black list
#ifdef PIDCALIBRATION
                    if (curr != pscreen_PID) {
#endif //PIDCALIBRATION
                        while (curr != pscreen_printing && curr != pscreen_home && curr != pscreen_menu_tune) {
                            screen_close();
                            curr = screen_get_curr();
                        }
#ifdef PIDCALIBRATION
                    }
#endif //PIDCALIBRATION
                }
                gui_timer_delete(gui_timeout_id);
            }
        }
        if (marlin_event_clr(MARLIN_EVT_CommandBegin)) {
            if (marlin_command() == MARLIN_CMD_M600) {
                _dbg("M600 start");
                while (marlin_event_clr(MARLIN_EVT_UserConfirmRequired) == 0) {
                    gui_loop();
                }
                gui_msgbox("Change filament and press to continue", MSGBOX_BTN_OK);
                marlin_set_wait_user(0);
                while (marlin_event_clr(MARLIN_EVT_HostPrompt) == 0) {
                    gui_loop();
                }
                marlin_host_button_click(HOST_PROMPT_BTN_Continue);
            }
        }
#endif //LCDSIM
    }
}

void update_firmware_screen(void)
{
    font_t* font = resource_font(IDR_FNT_SPECIAL);
    font_t* font1 = resource_font(IDR_FNT_NORMAL);
    display->fill_rect(rect_ui16(0, 0, 240, 320), COLOR_BLACK);
    render_icon_align(rect_ui16(70, 20, 100, 100), IDR_PNG_icon_pepa, COLOR_BLACK, RENDER_FLG(ALIGN_CENTER, 0));
    display->draw_text(rect_ui16(10, 115, 240, 60), "Hi, this is your\nOriginal Prusa MINI.", font, COLOR_BLACK, COLOR_WHITE);
    display->draw_text(rect_ui16(10, 160, 240, 80), "Please insert the USB\ndrive that came with\nyour MINI and reset\nthe printer to flash\nthe firmware", font, COLOR_BLACK, COLOR_WHITE);
    render_text_align(rect_ui16(5, 250, 230, 40), "RESET PRINTER", font1, COLOR_ORANGE, COLOR_WHITE, padding_ui8(2, 6, 2, 2), ALIGN_CENTER);
    while (1) {
        if (jogwheel_button_down > 50)
            sys_reset();
        osDelay(1);
#ifndef _DEBUG
        HAL_IWDG_Refresh(&hiwdg); //watchdog reset
#endif //_DEBUG
    }
}
