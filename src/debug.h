#ifndef __DEBUG__H__
#define __DEBUG__H__
#include <Arduino.h>

#define DBG_LEVEL 4

// DUMMY
#define DBG_LOGE(DEBUG_ITEM)
#define DBG_LOGW(DEBUG_ITEM)
#define DBG_LOGI(DEBUG_ITEM)
#define DBG_LOGD(DEBUG_ITEM)
#define DBG_LOGD(DEBUG_ITEM)
#define DBG_LOGV(DEBUG_ITEM)

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

#define DEBUG_PRINT(DEBUG_ITEM) Serial.print(DEBUG_ITEM)
#define DEBUG_PRINTLN(DEBUG_ITEM) Serial.println(DEBUG_ITEM)
#define DEBUG_ATTR "[" + String(millis()) + ":" STRINGIZE(__LINE__)
#define DEBUG_ATTR_END "]:\t"
#define DEBUG_VAR(DEBUG_ITEM) DEBUG_PRINT(DEBUG_ATTR ":" #DEBUG_ITEM DEBUG_ATTR_END), DEBUG_PRINTLN(DEBUG_ITEM)
#define DEBUG_PRT(DEBUG_ITEM) DEBUG_PRINT(DEBUG_ATTR DEBUG_ATTR_END), DEBUG_PRINTLN(DEBUG_ITEM)

#if (DBG_LEVEL > 0)
#define DBG_LOGE(DEBUG_ITEM)    DEBUG_VAR(DEBUG_ITEM)
#define PRT_LOGE(DEBUG_ITEM)    DEBUG_PRT(DEBUG_ITEM)
#endif

#if (DBG_LEVEL > 1)
#define DBG_LOGW(DEBUG_ITEM)    DEBUG_VAR(DEBUG_ITEM)
#define PRT_LOGW(DEBUG_ITEM)    DEBUG_PRT(DEBUG_ITEM)
#endif

#if (DBG_LEVEL > 2)
#define DBG_LOGI(DEBUG_ITEM)    DEBUG_VAR(DEBUG_ITEM)
#define PRT_LOGI(DEBUG_ITEM)    DEBUG_PRT(DEBUG_ITEM)
#endif

#if (DBG_LEVEL > 3)
#define DBG_LOGD(DEBUG_ITEM)    DEBUG_VAR(DEBUG_ITEM)
#define PRT_LOGD(DEBUG_ITEM)    DEBUG_PRT(DEBUG_ITEM)
#endif

#if (DBG_LEVEL > 4)
#define DBG_LOGV(DEBUG_ITEM)    DEBUG_VAR(DEBUG_ITEM)
#define PRT_LOGV(DEBUG_ITEM)    DEBUG_PRT(DEBUG_ITEM)
#endif


#endif  //!__DEBUG__H__
// ESP_LOGE - error (lowest)

// ESP_LOGW - warning

// ESP_LOGI - info

// ESP_LOGD - debug

// ESP_LOGV