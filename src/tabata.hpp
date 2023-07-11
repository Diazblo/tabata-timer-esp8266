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
// Function to start the timer
bool startTimer(TimerSettings t_timer)
{   
    if(tabata.timerRunning || tabata.timerPaused){
        pauseTimer();
        return 0;
    }
    resetTimer(1);

    tabata.timer = t_timer;

    tabata.timerRunning = true;
    SET_STATE(COUNTDOWN, tabata.timer.initialCountdown);

    Serial.println("Tabata Timer Started");
    return 1;
}
void startTimer(uint8_t t_preset = 0)
{
    if (t_preset != 0)
    {
        tabata.preset = t_preset;
    }
    startTimer(timerEeprom.timers[tabata.preset]);
}



// Function to stop the timer
void stopTimer()
{
    SET_STATE(COMPLETED, 0);
    resetTimer();
    tabata.timerRunning = false;
    Serial.println("Tabata Timer Stopped");
}

void sequenceStop()
{
    tabata.sequenceCounter = false;
    tabata.timerSequence = false;
    stopTimer();
}
void sequenceNext()
{
    uint8_t presetIndex = tabata.sequence[tabata.sequenceCounter];
    if ((presetIndex < MAX_PRESETS) && presetIndex && tabata.sequenceCounter < ARR_SIZE(tabata.sequence))
    {
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
    if (!tabata.timerRunning || !tabata.timerPaused)
    {
        Serial.println("SEQUENCE");

        tabata.timerSequence = true;
        tabata.sequenceCounter = 0;

        LOAD_ARR(tabata.sequence, timerEeprom.warmUpSequence);
        LOAD_ARR(tabata.sequence, timerEeprom.basicSequence);
        LOAD_ARR(tabata.sequence, timerEeprom.regularSequence);

        sequenceNext();
    }
    else{
        pauseTimer();
    }
}

void sequenceLoop()
{
    if (tabata.timerSequence && tabata.Phase == COMPLETED)
    {
        sequenceNext();
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
            SET_STATE(WORK, tabata.timer.workTime);
            break;

        case WORK:
            SET_STATE(REST, tabata.timer.restTime);
            break;
        case REST:
            tabata.set++;
            if (tabata.set <= tabata.timer.sets)
            {
                SET_STATE(WORK, tabata.timer.workTime);
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
                SET_STATE(WORK, tabata.timer.workTime);
            }
            else
            {
                SET_STATE(COMPLETED, 0);
            }
            break;

        case COMPLETED:
            if (tabata.timerRunning)
                stopTimer();
            tabata.timerRunning = false;
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
