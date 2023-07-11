#ifndef TABATA_H
#define TABATA_H

#include "preprocessor_helper.h"
#include <Arduino.h>

#define MAX_PRESETS 11
#define MAX_SEQUENCE MAX_PRESETS * 3

#define STATE_RUNNING "RUNNING"
#define STATE_PAUSED "PAUSED"
#define STATE_STOPPED "STOPED"
#define STATE_SEQUENCE "SEQUENCE"

// Timer settings struct
struct TimerSettings
{
    // char name[20];
    int16_t initialCountdown;
    int16_t workTime;     // Work time in milliseconds
    int16_t restTime;     // Rest time in milliseconds
    int16_t recoveryTime; // Recovery time in milliseconds
    int16_t sets;         // Number of sets
    int16_t cycles;       // Number of cycles
};
TimerSettings timer;

// Timer Phases enum
enum TimerPhase {
    COUNTDOWN,
    BEGIN,
    REST,
    RECOVERY,
    DONE
};

struct TimerCurrent
{
    uint8_t preset;
    String state;
    String phase;
    uint8_t set;   // Current set
    uint8_t cycle; // Current cycle
    bool timerRunning;     // Timer running flag
    bool timerPaused;      // Timer paused flag
    bool timerSequence;    // Timer sequential run flag
    
    uint16_t countDown;
    uint16_t countTime;
    uint16_t elapsed;
    
    TimerPhase Phase;
    TimerSettings timer;
    TimerSettings timers[MAX_PRESETS];
    uint8_t sequenceCounter;
    uint8_t sequence[MAX_SEQUENCE];

};
TimerCurrent tabata;

struct TimerEeprom
{
    TimerSettings timers[MAX_PRESETS];
    uint8_t preset;
    uint8_t warmUpSequence[MAX_PRESETS];
    uint8_t basicSequence[MAX_PRESETS];
    uint8_t regularSequence[MAX_PRESETS];
};
TimerEeprom timerEeprom;

#define TIMER_LIVE_JSON(SETTINGS_STRUCT) \
    "{" \
    _JSON_FIELD_INT(SETTINGS_STRUCT, preset)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, set)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, cycle)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, countDown)\
    _JSON_FIELD_STRING(SETTINGS_STRUCT, state)\
    _JSON_FIELD_STRING_(SETTINGS_STRUCT, phase)\
    "}"

String get_live_json(){
    String message = TIMER_LIVE_JSON(tabata);
    return message;
}

#endif