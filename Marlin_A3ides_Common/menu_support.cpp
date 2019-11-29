// menu_support.cpp

#ifndef _EXTUI

#include "sys.h"
#include "../Marlin/src/lcd/menu/menu.h"


void menu_support()
{
	START_MENU();
	MENU_BACK(MSG_MAIN);
//                 01234567890123456789
//	STATIC_ITEM_P("");
#if (BOARD == A3IDES2209_REV01)
	STATIC_ITEM_P("Marlin-A3ides 2209");
#elif (BOARD == A3IDES2209_REV02)
	STATIC_ITEM_P("Marlin-A3ides 2209-2");
#elif (BOARD == A3IDES2130_REV01)
	STATIC_ITEM_P("Marlin-A3ides 2130");
#else
	STATIC_ITEM_P("Marlin - A3ides ????");
#endif
	char version[32];
	char build[32];
	int ver_maj = FW_VERSION / 100;
	int ver_min = (FW_VERSION - 100 * ver_maj) / 10;
	int ver_sub = FW_VERSION % 10;
	const char* stages[] = {"pre-alpha", "alpha", "beta", "RC", "final"};
	sprintf(version, " %d.%d.%d %s", ver_maj, ver_min, ver_sub, (char*)stages[FW_STAGENR]);
	const char* printers[] = {"???", "???", "MINI"};
	unsigned int printer_type = PRINTER_TYPE;
	if (printer_type > PRINTER_PRUSA_MINI) printer_type = 0;
#ifdef _DEBUG
	sprintf(build, " %d%s (DEBUG_%s)", (int)FW_BUILDNR, (char*)FW_BUILDSX, printers[printer_type]);
#else //_DEBUG
	sprintf(build, " %d%s (%s)", (int)FW_BUILDNR, (char*)FW_BUILDSX, printers[printer_type]);
#endif //_DEBUG
	STATIC_ITEM_P("version:            ");
	STATIC_ITEM(version);
	STATIC_ITEM_P("build:            ");
	STATIC_ITEM(build);
	END_MENU();
}

#endif //_EXTUI
