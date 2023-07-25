#include "tabata.hpp"
#include "tabata.h"
#include "preprocessor_helper.h"
#include "interactor.h"
#include "display.h"
#include "webserver.hpp"

int16_t variable = 0;
int16_t variable_pos = 0;
bool variable_flag = 0;

enum MenuState
{
    HOME,
    SETANDSTART
} menuState;

struct displayFlag
{
    String messages[10];
    String lastMessage[10];
    uint8_t infoCounter;
    unsigned long infoMillis;
} screen;

InteractorAction interactorAction;

#define TIMER_VALUES()                            \
    "{" JSON_FIELD_STRING("P", tabata.phase)      \
        JSON_FIELD_INT("T", tabata.elapsed)       \
            JSON_FIELD_INT("S", tabata.set)       \
                JSON_FIELD_INT("C", tabata.cycle) \
                    JSON_FIELD_INT_("I", tabata.preset) "}"

#define TIMER_SETTINGS(TIMER_STRUCT)                        \
    "{" _JSON_FIELD_INT(TIMER_STRUCT, initialCountdown)     \
        _JSON_FIELD_INT(TIMER_STRUCT, workTime)             \
            _JSON_FIELD_INT(TIMER_STRUCT, restTime)         \
                _JSON_FIELD_INT(TIMER_STRUCT, recoveryTime) \
                    _JSON_FIELD_INT_(TIMER_STRUCT, sets)    \
                        _JSON_FIELD_INT_(TIMER_STRUCT, cycles) "}"

void setup()
{
    Serial.begin(115200);
    tabata_init();
    display_init();
    interactor_init();
    loadEeprom();
    webserver_init();
}
void loop()
{
    INTERVAL_TIMER(50)
    {
        interactor_loop();
    }
    INTERVAL_TIMER(150)
    {
        interactorAction = interactor_get();
        serialLoop();
        interactorMenu();
    }
    tabata_loop();
    webserver_loop();
}
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

byte phaseColor(TimerPhase t_phase)
{
    switch (t_phase)
    {
    case COUNTDOWN:
        return BAR_WHITE;
        break;
    case BEGIN:
        return BAR_RED;
        break;
    case REST:
        return BAR_GREEN;
        break;
    case RECOVERY:
        return BAR_BLACK;
        break;
    case DONE:
        return BAR_BLACK;
        break;
    }
    return 0;
}

void displayMain(TimerCurrent t_timer)
{
    if (screen.infoCounter)
    {
        display_text(screen.messages[screen.infoCounter - 1]);
        if (millis() - screen.infoMillis > 1000)
        {
            screen.infoCounter--;
            screen.infoMillis = millis();
        }
    }
    else
    {   
        uint16_t timer_remaining = t_timer.countTime - t_timer.elapsed;
        display_bar(timer_remaining, phaseColor(t_timer.Phase), true, t_timer.countTime);
        displayInfoAdd(tabata.phase,2);

        if(t_timer.timerRunning){
            // Timer Running
            display_time(timer_remaining, (timer_remaining<5) ? true:false);
            displayInfoAdd("Pre" + String(tabata.preset),1);  
        }else if(!t_timer.timerPaused){
            // Timer Idle
            display_text("  -  ");
        }else if(t_timer.timerPaused){
            // Timer Paused
            displayInfoAdd("PAUSED",1);
        }
    }
}

void displayInfoAdd(String t_message, uint8_t t_index)
{
    if (!(t_message == screen.lastMessage[t_index]))
    {
        beep(1);
        screen.lastMessage[t_index] = t_message;
        screen.messages[screen.infoCounter++] = t_message;
        screen.infoMillis = millis();
        if (screen.infoCounter == ARR_SIZE(screen.messages))
            screen.infoCounter = 0;
    }
}

void interactorMenu()
{
    switch (menuState)
    {
    case HOME:
        interactorOutput.buffer = TIMER_VALUES();
        displayMain(tabata);

        if (interactorAction == PRESS){
            playTimer(-1);
        }
        if (interactorAction == LONGPRESS)
            menuState = SETANDSTART;
        if (interactorAction == PRESSPRESS)
        {
            playTimer(-2);
        }
        if (interactorAction == LONGLONGPRESS)
        {
            deviceSleep();
        }
        break;

    case SETANDSTART:
        variable += (variable_flag ? 60:1) * processPos(interactorAction);
        if(variable < 0) variable = 0;

        if (interactorAction == PRESS){
            variable_flag = !variable_flag;
        }

        if (interactorAction == LONGPRESS)
        {
            sequenceStop();
            stopTimer();
            startTimer({10, variable, 0, 0, 0, 0});
            menuState = HOME;
        }

        interactorOutput = {String(variable), variable_pos};
        display_time(variable,variable_flag+2);
        display_bar((variable_flag ? 6:2), B100, true);
        break;
    }

    if (interactorAction != NONE)
        interactorAction = NONE;

    // Serial.println(String(interactorOutput.position) + ":" + interactorOutput.buffer);
    // return output_buffer;
}


// Function to simulate parts of code through serial
void serialLoop()
{
    uint8_t incoming = Serial.read();
    if (incoming)
    {
        switch (incoming)
        {
            //
        case 's':
            startTimer();
            break;
        case 'S':
            sequenceStart();
            break;
        case 'p':
            pauseTimer();
            break;
        case 'x':
            sequenceStop();
            break;
        case 'n':
            sequenceNext();
            break;
        case 'e':
            Serial.println(TIMER_SETTINGS(timerEeprom.timers[1]));
            ASSIGN_ARR(timerEeprom.warmUpSequence, "1,2,1");
            ASSIGN_ARR(timerEeprom.basicSequence, "4,5");
            ASSIGN_ARR(timerEeprom.regularSequence, "3");
            timerEeprom.preset = 1;
            for (uint8_t i = 0; i < 11; i++)
            {
                if (i == 2)
                    assignEepromTimer({10, 10, 10, 20, 2, 2}, i);
                else
                    assignEepromTimer({4, 1, 1, 1, 1, 1}, i);
            }
            updateEeprom();
            break;
        case 'q':
            startTimer({10, 0, 0, 0, 0, 0});
            break;
        case 'w':
            startTimer(2);
            break;
        case '1':
            interactorAction = PRESS;
            break;
        case '2':
            interactorAction = LONGPRESS;
            break;
        case '3':
            interactorAction = UP;
            break;
        case '4':
            interactorAction = DOWN;
            break;
        case '5':
            interactorAction = PRESSPRESS;
            break;
        case '6':
            interactorAction = LONGLONGPRESS;
            break;
        case 't':
            displayInfoAdd("HIII",0);
            break;
        case 'b':
            deviceSleep();
            break;
        case 'U':
            playTimer();
        break;
        case 'u':
            playTimer(-1);
        break;
        }
        Serial.flush();
    }
}
