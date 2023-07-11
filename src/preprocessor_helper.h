#ifndef __PREPROCESSOR_HELPER__H__
#define __PREPROCESSOR_HELPER__H__

#define ARR_SIZE(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#define ITER_ARR_R(ARRAY_NAME, RESULT_VAR)                 \
    static uint8_t iter_counter_##ARRAY_NAME = 0;          \
    iter_counter_##ARRAY_NAME++;                           \
    if (iter_counter_##ARRAY_NAME == ARR_SIZE(ARRAY_NAME)) \
        iter_counter_##ARRAY_NAME = 0;                     \
    RESULT_VAR = ARRAY_NAME[iter_counter_##ARRAY_NAME];

#define ITER_ARR(ARRAY_NAME)                               \
    static uint8_t iter_counter_##ARRAY_NAME = 0;          \
    iter_counter_##ARRAY_NAME++;                           \
    if (iter_counter_##ARRAY_NAME == ARR_SIZE(ARRAY_NAME)) \
        iter_counter_##ARRAY_NAME = 0;                     \
    uint8_t ITER_##ARRAY_NAME = ARRAY_NAME[iter_counter_##ARRAY_NAME];
// ARRAY_NAME[iter_counter_##ARRAY_NAME]

#define CLEAR_ARR(ARRAY)                                    \
    {                                                       \
        for (uint8_t i = 0; i < ARR_SIZE(ARRAY); i++) \
            ARRAY[i] = 0;                                   \
    }

#define LOAD_ARR(DESTINATION, SOURCE)                             \
    {                                                             \
        uint8_t load_index = 0;                                   \
        for (uint8_t i = 0; i < ARR_SIZE(DESTINATION); i++)       \
        {                                                         \
            if (DESTINATION[i] == 0 && (SOURCE[load_index] != 0)) \
            {                                                     \
                if (load_index < ARR_SIZE(SOURCE))                \
                {                                                 \
                    DESTINATION[i] = SOURCE[load_index++];        \
                }                                                 \
            }                                                     \
        }                                                         \
    }

#define ASSIGN_ARR(DESTINATION, STRING)                                           \
    {                                                                             \
        String source = STRING;                                                   \
        uint8_t index = 0;                                                        \
        while (index < ARR_SIZE(DESTINATION))                                     \
        {                                                                         \
            if (source.length() > 0)                                              \
            {                                                                     \
                int commaIndex = source.indexOf(',');                             \
                if (commaIndex >= 0)                                              \
                {                                                                 \
                    DESTINATION[index] = source.substring(0, commaIndex).toInt(); \
                    source = source.substring(commaIndex + 1);                    \
                }                                                                 \
                else                                                              \
                {                                                                 \
                    DESTINATION[index] = source.toInt();                          \
                    source = "";                                                  \
                }                                                                 \
            }                                                                     \
            else                                                                  \
            {                                                                     \
                DESTINATION[index] = 0;                                           \
            }                                                                     \
            index++;                                                              \
        }                                                                         \
    }

#define PRINT_ARR(ARRAY_NAME)                             \
    for (uint16_t i = 0; i < (ARR_SIZE(ARRAY_NAME)); i++) \
    {                                                     \
        Serial.print(String(ARRAY_NAME[i]) + ",");        \
    }                                                     \
    Serial.println();

#define JSON_FIELD_STRING(name, value) "\"" name "\": \"" + value + "\","
#define JSON_FIELD_INT(name, value) "\"" name "\": " + String(value) + ","

#define JSON_FIELD_STRING_(name, value) "\"" name "\": \"" + value + "\""
#define JSON_FIELD_INT_(name, value) "\"" name "\": " + String(value) +

// These just take struct name and variable name to create json string
#define _JSON_FIELD_STRING(SETTINGS_STRUCT, name) "\"" #name "\": \"" + SETTINGS_STRUCT.name + "\","
#define _JSON_FIELD_INT(SETTINGS_STRUCT, name) "\"" #name "\": " + String(SETTINGS_STRUCT.name) + ","

#define _JSON_FIELD_STRING_(SETTINGS_STRUCT, name) "\"" #name "\": \"" + SETTINGS_STRUCT.name + "\""
#define _JSON_FIELD_INT_(SETTINGS_STRUCT, name) "\"" #name "\": " + String(SETTINGS_STRUCT.name) +

#define ST_VAR(SETTINGS_STRUCT, VAR) #VAR, SETTINGS_STRUCT.VAR

#define INTERVAL_TIMER(INTERVAL)                                \
    static unsigned long interval_timer_millis_##INTERVAL = 0;  \
    bool interval_timer_bool_##INTERVAL = 0;                    \
    if (millis() - interval_timer_millis_##INTERVAL > INTERVAL) \
    {                                                           \
        interval_timer_millis_##INTERVAL = millis();            \
        interval_timer_bool_##INTERVAL = 1;                     \
    }                                                           \
    if (interval_timer_bool_##INTERVAL)

#endif  //!__PREPROCESSOR_HELPER__H__