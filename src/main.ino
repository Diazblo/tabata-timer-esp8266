#include "tabata.hpp"
#include "tabata.h"
#include "preprocessor_helper.h"
#include "interactor.h"
#include "display.h"

int16_t variable = 0;
int16_t variable_pos = 0;
bool variable_flag = 1;


enum MenuState
{
    HOME,
    SETANDSTART
} menuState;

InteractorAction interactorAction;

#define TIMER_VALUES()                            \
    "{" JSON_FIELD_STRING("P", tabata.phase)      \
        JSON_FIELD_INT("T", tabata.elapsed)       \
            JSON_FIELD_INT("S", tabata.set)       \
                JSON_FIELD_INT("C", tabata.cycle) \
                    JSON_FIELD_INT_("I", tabata.preset) "}"


void setup()
{
    Serial.begin(115200);
    tabata_init();
    display_init();
    interactor_init();
}
void loop()
{   
    INTERVAL_TIMER(50){
        interactor_loop();
    }
    INTERVAL_TIMER(100)
    {
        interactorAction = interactor_get();
        serialLoop();
        interactorMenu();
    }
    tabata_loop();
}

byte phaseColor(TimerPhase t_phase){
    switch (t_phase)
    {
        case COUNTDOWN:
            return BAR_WHITE;
            break;
        case WORK:
            return BAR_GREEN;
            break;
        case REST:
            return BAR_YELLOW;
            break;
        case RECOVERY:
            return BAR_RED;
            break;   
    }
    return 0;
}

void display_main(TimerCurrent t_timer ){
    display_time(t_timer.countTime-t_timer.elapsed);
    display_bar(t_timer.countTime-t_timer.elapsed, phaseColor(t_timer.Phase), true, t_timer.countTime);
}

void interactorMenu()
{
    switch (menuState)
    {
    case HOME:
        interactorOutput.buffer = TIMER_VALUES();
        display_main(tabata);

        if (interactorAction == PRESS)
            sequenceStart();
        if (interactorAction == LONGPRESS)
            menuState = SETANDSTART;
        if (interactorAction == LONGLONGPRESS)
        {
            sequenceStop();
        }
        break;

    case SETANDSTART:
        if (variable_flag)
        {
            variable += pow(10, variable_pos) * processPos(interactorAction);
        }
        else
        {
            variable_pos += processPos(interactorAction);
            if (variable_pos < 0)
                variable_pos = 0;
        }

        if (interactorAction == PRESS)
            variable_flag = !variable_flag;

        if (interactorAction == LONGPRESS)
        {
            startTimer({variable, 0, 0, 0, 0, 0});
            menuState = HOME;
        }

        interactorOutput = {String(variable), variable_pos};
        display_seconds(variable);
        display_bar(variable_pos+1, B111, true);
        break;
    }

    if (interactorAction != NONE)
        interactorAction = NONE;

    Serial.println(String(interactorOutput.position) + ":" + interactorOutput.buffer);
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
            startTimer({4, 5, 5, 8, 3, 3});
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
            timerEeprom.timers[0] = {5, 5, 5, 5, 2, 2};
            timerEeprom.timers[1] = {5, 5, 5, 5, 2, 2};
            timerEeprom.timers[2] = {5, 5, 5, 5, 2, 2};
            timerEeprom.timers[3] = {5, 5, 5, 5, 2, 2};
            timerEeprom.timers[4] = {5, 5, 5, 5, 2, 2};
            timerEeprom.timers[5] = {5, 5, 5, 5, 2, 2};
            ASSIGN_ARR(timerEeprom.warmUpSequence, "1,2,1");
            ASSIGN_ARR(timerEeprom.basicSequence, "4,5");
            ASSIGN_ARR(timerEeprom.regularSequence, "3");
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
        }
        Serial.flush();
    }
}
