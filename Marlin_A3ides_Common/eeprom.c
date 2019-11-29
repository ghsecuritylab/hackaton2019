// eeprom.c

#include "eeprom.h"
#include <stdio.h>
#include "st25dv64k.h"
#include "dbg.h"
#include <string.h>

#define EE_VERSION 0x0001
#define EE_VAR_CNT sizeof(eeprom_map_v1)
#define EE_ADDRESS 0x0500

const uint8_t eeprom_map_v1[] = {
    VARIANT8_UI16, // EEVAR_VERSION
    VARIANT8_UI8, // EEVAR_FILAMENT_TYPE
    VARIANT8_UI32, // EEVAR_FILAMENT_COLOR
};

const char* eeprom_var_name[] = {
    "VERSION",
    "FILAMENT_TYPE",
    "FILAMENT_COLOR",
};

uint16_t eeprom_crc_value = 0;
uint8_t eeprom_crc_index = 0;

// forward declarations of private functions

uint16_t eeprom_var_size(uint8_t id);
uint16_t eeprom_var_addr(uint8_t id);
variant8_t eeprom_var_default(uint8_t id);
void eeprom_dump(void);
void eeprom_print_vars(void);
void eeprom_clear(void);

// public functions

uint8_t eeprom_init(void)
{
    uint8_t id;
    uint8_t ret = 0;
    st25dv64k_init();
    //eeprom_clear();
    //eeprom_dump();
    uint16_t version = eeprom_get_var(EEVAR_VERSION).ui16;
    if (version != EE_VERSION) {
        for (id = 0; id < EE_VAR_CNT; id++)
            eeprom_set_var(id, eeprom_var_default(id));
        ret = 1;
    }
    //eeprom_print_vars();
    //eeprom_dump();
    return ret;
}

variant8_t eeprom_get_var(uint8_t id)
{
    uint16_t addr;
    uint16_t size;
    variant8_t var = variant8_empty();
    if (id < EE_VAR_CNT) {
        var.type = eeprom_map_v1[id];
        addr = eeprom_var_addr(id);
        size = eeprom_var_size(id);
        st25dv64k_user_read_bytes(addr, &(var.ui32), size);
    }
    return var;
}

void eeprom_set_var(uint8_t id, variant8_t var)
{
    uint16_t addr;
    uint16_t size;
    if (id < EE_VAR_CNT) {
        if (var.type == eeprom_map_v1[id]) {
            addr = eeprom_var_addr(id);
            size = eeprom_var_size(id);
            st25dv64k_user_write_bytes(addr, &(var.ui32), size);
            //			osDelay(5);
        }
    }
}

// private functions

uint16_t eeprom_var_size(uint8_t id)
{
    if (id < EE_VAR_CNT)
        switch (eeprom_map_v1[id]) {
        case VARIANT8_I8:
        case VARIANT8_UI8:
            return 1;
        case VARIANT8_I16:
        case VARIANT8_UI16:
            return 2;
        case VARIANT8_I32:
        case VARIANT8_UI32:
        case VARIANT8_FLT:
            return 4;
        }
    return 0;
}

uint16_t eeprom_var_addr(uint8_t id)
{
    uint16_t addr = EE_ADDRESS;
    while (id)
        addr += eeprom_var_size(--id);
    return addr;
}

variant8_t eeprom_var_default(uint8_t id)
{
    switch (id) {
    case EEVAR_VERSION:
        return variant8_ui16(EE_VERSION);
    case EEVAR_FILAMENT_TYPE:
        return variant8_ui8(0);
    case EEVAR_FILAMENT_COLOR:
        return variant8_ui32(0);
    }
    return variant8_empty();
}

void eeprom_dump(void)
{
    int i;
    int j;
    uint8_t b;
    char line[64];
    for (i = 0; i < 128; i++) // 128 lines = 2048 bytes
    {
        sprintf(line, "%04x", i * 16);
        for (j = 0; j < 16; j++) {
            b = st25dv64k_user_read(j + i * 16);
            sprintf(line + 4 + j * 3, " %02x", b);
        }
        _dbg("%s", line);
    }
}

int eeprom_var_sprintf(char* str, uint8_t id, variant8_t var)
{
    switch (id) {
    case EEVAR_VERSION:
        return sprintf(str, "%u", var.ui16);
    case EEVAR_FILAMENT_TYPE:
        return sprintf(str, "%u", var.ui8);
    case EEVAR_FILAMENT_COLOR:
        return sprintf(str, "0x%08lx", var.ui32);
    }
    return 0;
}

void eeprom_print_vars(void)
{
    uint8_t id;
    char text[16];
    for (id = 0; id < EE_VAR_CNT; id++) {
        eeprom_var_sprintf(text, id, eeprom_get_var(id));
        _dbg("%s=%s", eeprom_var_name[id], text);
    }
}

void eeprom_clear(void)
{
    uint16_t a;
    uint32_t data = 0xffffffff;
    for (a = 0x0000; a < 0x0800; a += 4)
        st25dv64k_user_write_bytes(a, &data, 4);
}

int8_t eeprom_test_PUT(const unsigned int bytes)
{

    unsigned int i;
    char line[16] = "abcdefghijklmno";
    char line2[16];
    uint8_t size = sizeof(line);
    unsigned int count = bytes / 16;

    for (i = 0; i < count; i++) {
        st25dv64k_user_write_bytes(EE_ADDRESS + i * size, &line, size);
    }

    int8_t res_flag = 1;

    for (i = 0; i < count; i++) {
        st25dv64k_user_read_bytes(EE_ADDRESS + i * size, &line2, size);
        if (strcmp(line2, line))
            res_flag = 0;
    }
    return res_flag;
}
