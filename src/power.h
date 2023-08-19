#ifndef __POWER__H__
#define __POWER__H__
#include <Arduino.h>
#include "board_defs.h"
#include "preprocessor_helper.h"

void deviceSleep()
{   
    size_t pin_count = ARR_SIZE(board_pins_array);
    for (uint8_t i = 0; i < pin_count; i++)
    {
        pinMode(board_pins_array[i], INPUT);
    }
    display_sleep();
    ESP.deepSleep(0);
}

int getBatteryVoltage(){
    return 5.51 * analogRead(BATTERY_PIN);
}
int getBattery(){
    int battery_level = constrain(getBatteryVoltage(), 3300, 4200);
    battery_level = map(battery_level, 3300, 4200, 0, 100);
    
    return battery_level;
}

#define STATS_JSON() \
    "{" \
    _JSON_FIELD_FUNCTION(getBattery)\
    _JSON_FIELD_FUNCTION(getBatteryVoltage)\
    _JSON_FIELD_FUNCTION(WiFi.getMode)\
    _JSON_FIELD_FUNCTION(WiFi.macAddress)\
    _JSON_FIELD_FUNCTION(WiFi.RSSI)\
    _JSON_FIELD_FUNCTION(WiFi.status)\
    _JSON_FIELD_FUNCTION(ESP.getFreeHeap)\
    _JSON_FIELD_FUNCTION(ESP.getVcc)\
    _JSON_FIELD_FUNCTION(ESP.getFlashChipRealSize)\
    _JSON_FIELD_FUNCTION(ESP.getFlashChipSize)\
    _JSON_FIELD_FUNCTION(ESP.getFlashChipMode)\
    _JSON_FIELD_FUNCTION(ESP.getResetReason)\
    _JSON_FIELD_FUNCTION(system_get_sdk_version)\
    _JSON_FIELD_FUNCTION(system_get_boot_version)\
    _JSON_FIELD_FUNCTION_(system_get_userbin_addr)\
    "}"

String get_stats_json(){
    return STATS_JSON();
}

#endif  //!__POWER__H__