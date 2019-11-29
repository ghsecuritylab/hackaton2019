//appmain.cpp - arduino-like app start

#include "app.h"
#include "dbg.h"
#include "cmsis_os.h"
#include "config.h"
#include "dbg.h"
#include "adc.h"
#include "jogwheel.h"
#include "hwio_a3ides.h"
#include "sys.h"

#ifdef SIM_HEATER
#include "sim_heater.h"
#endif //SIM_HEATER

#ifdef SIM_MOTION
#include "sim_motion.h"
#endif //SIM_MOTION

#include "uartslave.h"
#include "marlin_server.h"
#include "bsod.h"
#include "eeprom.h"
#include "diag.h"

#include <Arduino.h>


#define DBG _dbg0 //debug level 0
//#define DBG(...)  //disable debug


extern void USBSerial_put_rx_data(uint8_t *buffer, uint32_t length);


extern "C"
{

extern void init_tmc(void);

extern uartrxbuff_t uart6rxbuff;       // PUT rx buffer
extern uartslave_t uart6slave;         // PUT slave

#ifdef ETHERNET
extern osThreadId webServerTaskHandle; // Webserver thread(used for fast boot mode)
#endif //ETHERNET

#ifndef _DEBUG
extern IWDG_HandleTypeDef hiwdg; //watchdog handle
#endif //_DEBUG


void app_setup(void)
{
	setup();

	init_tmc();
	//DBG("after init_tmc (%ld ms)", HAL_GetTick());
}

void app_idle(void)
{
	osDelay(0); // switch to other threads - without this is UI slow during printing
}

void app_run(void)
{
	DBG("app_run");

#ifdef ETHERNET
	if(diag_fastboot)
		osThreadResume(webServerTaskHandle);
#endif //ETHERNET

	eeprom_init();

	marlin_server_init();
	marlin_server_idle_cb = app_idle;

	adc_init();

#ifdef SIM_HEATER
	sim_heater_init();
#endif //SIM_HEATER

	//DBG("before setup (%ld ms)", HAL_GetTick());
	if (diag_fastboot || (!sys_fw_is_valid()))
		marlin_server_stop_processing();
	else
		app_setup();
	//DBG("after setup (%ld ms)", HAL_GetTick());

	while (1)
	{
		if (marlin_server_processing())
		{
			loop();
		}
		uartslave_cycle(&uart6slave);
		marlin_server_loop();
		osDelay(0); // switch to other threads - without this is UI slow
#ifdef JOGWHEEL_TRACE
		static int signals = jogwheel_signals;
		if (signals != jogwheel_signals)
		{
			signals = jogwheel_signals;
			DBG("%d %d", signals, jogwheel_encoder);
		}
#endif //JOGWHEEL_TRACE
#ifdef SIM_MOTION_TRACE_X
		static int32_t x = sim_motion_pos[0];
		if (x != sim_motion_pos[0])
		{
			x = sim_motion_pos[0];
			DBG("X:%li", x);
		}
#endif //SIM_MOTION_TRACE_X
#ifdef SIM_MOTION_TRACE_Y
		static int32_t y = sim_motion_pos[1];
		if (y != sim_motion_pos[1])
		{
			y = sim_motion_pos[1];
			DBG("Y:%li", y);
		}
#endif //SIM_MOTION_TRACE_Y
#ifdef SIM_MOTION_TRACE_Z
		static int32_t z = sim_motion_pos[2];
		if (z != sim_motion_pos[2])
		{
			z = sim_motion_pos[2];
			DBG("Z:%li", z);
		}
#endif //SIM_MOTION_TRACE_Z
	}

}

void app_error(void)
{
	bsod("app_error");
}

void app_assert(uint8_t *file, uint32_t line)
{
	bsod("app_assert");
}

void app_cdc_rx(uint8_t *buffer, uint32_t length)
{
	USBSerial_put_rx_data(buffer, length);
}

void app_tim6_tick(void)
{
	adc_cycle();
#ifdef SIM_HEATER
	static uint8_t cnt_sim_heater = 0;
	if (++cnt_sim_heater >= 50) // sim_heater freq = 20Hz
	{
		sim_heater_cycle();
		cnt_sim_heater = 0;
	}
#endif //SIM_HEATER
#ifdef SIM_MOTION
	sim_motion_cycle();
#endif //SIM_MOTION
}

void app_tim14_tick(void)
{
	jogwheel_update_1ms();
	hwio_update_1ms();
}


} // extern "C"

//cpp code