#include "preprocessor_helper.h"
#include "tabata.h"
#include <Arduino.h>

unsigned long startTime; // Start time for the current state

#define SET_STATE(T_STATE, T_TOTAL_TIME) setState(T_STATE, T_TOTAL_TIME, #T_STATE)

// Function to change state
void setState(TimerPhase t_state, uint16_t t_total_time, String t_phase_string = "")
{
    tabata.Phase = t_state;
    startTime = (millis() / 1000);
    tabata.phase = t_phase_string;
    tabata.countTime = t_total_time;
}

bool timerRun()
{
    tabata.elapsed = (millis() / 1000) - startTime;
    tabata.countDown = tabata.countTime - tabata.elapsed;

    if (tabata.elapsed >= tabata.countTime)
    {
        return 1;
    }
    return 0;
}

// Function to reset the timer
void resetTimer(uint8_t t_default = 0)
{
    tabata.set = t_default;
    tabata.cycle = t_default;
    tabata.elapsed = 0;
}

// Function to pause the timer
void pauseTimer()
{
    if (tabata.timerRunning)
    {
        tabata.elapsed = (millis() / 1000) - startTime;
        tabata.timerRunning = false;
        tabata.timerPaused = true;
    }
    else if (tabata.timerPaused)
    {
        startTime = (millis() / 1000) - tabata.elapsed;
        tabata.timerRunning = true;
        tabata.timerPaused = false;
    }
}
#define TIMER_SETTINGS(TIMER_STRUCT)                        \
    "{" _JSON_FIELD_INT(TIMER_STRUCT, initialCountdown)     \
        _JSON_FIELD_INT(TIMER_STRUCT, workTime)             \
            _JSON_FIELD_INT(TIMER_STRUCT, restTime)         \
                _JSON_FIELD_INT(TIMER_STRUCT, recoveryTime) \
                    _JSON_FIELD_INT_(TIMER_STRUCT, sets)    \
                        _JSON_FIELD_INT_(TIMER_STRUCT, cycles) "}"
// Function to start the timer
bool startTimer(TimerSettings t_timer)
{   
    PRT_LOGD(TIMER_SETTINGS(t_timer));

    resetTimer(1);

    tabata.timer = t_timer;

    tabata.timerRunning = true;
    SET_STATE(COUNTDOWN, tabata.timer.initialCountdown);

    PRT_LOGI("Tabata Timer Started");
    return 1;
}
void startTimer(uint8_t t_preset = 0)
{
    if (t_preset != 0)
        tabata.preset = t_preset;
    else
        tabata.preset = timerEeprom.preset;

    startTimer(timerEeprom.timers[tabata.preset]);
}

int playTimer(int8_t t_index=0, uint16_t t_time=0){
    if (t_index == -2)
    {
        sequenceStop();
    }
    else if (tabata.timerRunning || tabata.timerPaused)
    {
        pauseTimer();
    }
    else if (t_index == -1)
    {
        sequenceStart();
    }
    else if(t_time >0)
    {
        tabata.preset = t_index;
        timerEeprom.timers[0] = {10,t_time,0,0,0,0};
        startTimer(timerEeprom.timers[tabata.preset]);
    }
    else if (t_index == 0)
    {
        tabata.preset = timerEeprom.preset;
        startTimer(timerEeprom.timers[tabata.preset]);
    }
    else
    {
        tabata.preset = t_index;
        startTimer(timerEeprom.timers[tabata.preset]);

    }
    return tabata.timerRunning | (tabata.timerPaused<<1);
}

// Function to stop the timer
void stopTimer()
{
    SET_STATE(DONE, 0);
    resetTimer();
    tabata.timerRunning = false;
    tabata.timerPaused = false;
    PRT_LOGI("Tabata Timer Stopped");
}

void sequenceStop()
{
    tabata.sequenceCounter = 0;
    tabata.timerSequence = false;
    stopTimer();
}
void sequenceNext()
{
    uint8_t presetIndex = tabata.sequence[tabata.sequenceCounter];
    if ((presetIndex < MAX_PRESETS) && presetIndex && tabata.sequenceCounter < ARR_SIZE(tabata.sequence))
    {   
        stopTimer();
        startTimer(presetIndex);
        tabata.sequenceCounter++;
    }
    else
    {
        sequenceStop();
    }
}

void sequenceStart()
{
    PRT_LOGD("SEQUENCE");

    tabata.timerSequence = true;
    tabata.sequenceCounter = 0;

    CLEAR_ARR(tabata.sequence);
    LOAD_ARR(tabata.sequence, timerEeprom.warmUpSequence);
    LOAD_ARR(tabata.sequence, timerEeprom.basicSequence);
    LOAD_ARR(tabata.sequence, timerEeprom.regularSequence);
    
    PRINT_ARR(tabata.sequence);

    sequenceNext();
}

void sequenceLoop()
{
    if (tabata.timerSequence && tabata.Phase == DONE && !tabata.timerRunning)
    {
        sequenceNext();
        PRT_LOGD("SEQUENCE NEXT");
    }
}

// Function to update the timer state
void updateTimerState()
{
    if (!tabata.timerRunning)
    {
        return;
    }

    if (timerRun())
        switch (tabata.Phase)
        {
        case COUNTDOWN:
            SET_STATE(BEGIN, tabata.timer.workTime);
            break;

        case BEGIN:
            SET_STATE(REST, tabata.timer.restTime);
            break;
        case REST:
            tabata.set++;
            if (tabata.set <= tabata.timer.sets)
            {
                SET_STATE(BEGIN, tabata.timer.workTime);
            }
            else
            {
                SET_STATE(RECOVERY, tabata.timer.recoveryTime);
                tabata.set = 1;
            }
            break;

        case RECOVERY:
            tabata.cycle++;
            if (tabata.cycle <= tabata.timer.cycles)
            {
                SET_STATE(BEGIN, tabata.timer.workTime);
            }
            else
            {
                SET_STATE(DONE, 0);
            }
            break;

        case DONE:
            if (tabata.timerRunning)
                stopTimer();
            break;
        }
}

void tabata_init()
{
    // startTimer(presets[0]);
}

void tabata_loop()
{
    updateTimerState();
    sequenceLoop();
}
