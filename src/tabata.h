#ifndef TABATA_H
#define TABATA_H

#include "preprocessor_helper.h"
#include <Arduino.h>
#include <EEPROM.h>

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
    int initialCountdown;
    int workTime;     // Work time in milliseconds
    int restTime;     // Rest time in milliseconds
    int recoveryTime; // Recovery time in milliseconds
    int sets;         // Number of sets
    int cycles;       // Number of cycles
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

void assignEeprom(TimerSettings t_timer, uint8_t t_index){
    timerEeprom.timers[t_index] = t_timer;
    timerEeprom.preset = t_index;
}
void updateEeprom()
{
    EEPROM.begin(sizeof(timerEeprom));
    EEPROM.put(0, timerEeprom);
    EEPROM.commit();
    EEPROM.end();
}
void loadEeprom()
{
    EEPROM.begin(sizeof(timerEeprom));
    EEPROM.get(0, timerEeprom);
    EEPROM.end();
    tabata.preset = timerEeprom.preset;
}

#define TIMER_LIVE_JSON(SETTINGS_STRUCT) \
    "{" \
    _JSON_FIELD_INT(SETTINGS_STRUCT, preset)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, set)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, cycle)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, countDown)\
    _JSON_FIELD_STRING(SETTINGS_STRUCT, state)\
    _JSON_FIELD_STRING_(SETTINGS_STRUCT, phase)\
    "}"

#define TIMER_SETINGS_JSON(SETTINGS_STRUCT) \
    "{" \
    _JSON_FIELD_INT(SETTINGS_STRUCT, initialCountdown)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, workTime)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, restTime)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, recoveryTime)\
    _JSON_FIELD_INT(SETTINGS_STRUCT, sets)\
    _JSON_FIELD_INT_(SETTINGS_STRUCT, cycles)\
    "}"

String get_live_json(){
    String message = TIMER_LIVE_JSON(tabata);
    return message;
}

String get_timers_json(){
    String message = "{\"EEPROM\":[";
    for(uint8_t i=0; i<MAX_PRESETS; i++){
        message += TIMER_SETINGS_JSON(timerEeprom.timers[i]);
        message += ",";
    }
    message = message.substring(0,message.length()-1);
    message += "]}";
    return message;
}


void pauseTimer();
void startTimer(uint8_t t_preset);
void stopTimer();
void sequenceStop();
void sequenceNext();
void sequenceStart();

#endif