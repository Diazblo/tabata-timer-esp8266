#include "tabata.hpp"
#include "tabata.h"
#include "preprocessor_helper.h"
#include "interactor.h"
#include "display.h"
#include "webserver.hpp"
#include "power.h"
#include "debug.h"

byte colorByPhase[]     ={BAR_WHITE, BAR_RED, BAR_GREEN, BAR_BLUE, BAR_BLACK};
String messageByPhase[] ={"COUNT", "BEGIN", "REST", "RECOVERY", "DONE"};

int16_t variable = 0;
int16_t variable_pos = 0;
bool variable_flag = 0;

bool onBootFlag = true;
bool indicateByPhaseOnce = false;
bool indicateByTick = false;
bool indicateByPauseOnce = true;

TimerPhase lastPhase;
uint16_t lastElapsed;

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
    display_init();
    checkBattery();
    display_text("  -  ");
    tabata_init();
    interactor_init();
    loadEeprom();
    webserver_init();
    DBG_LOGI(get_stats_json());
    boot_intro();
}
void loop()
{
    INTERVAL_TIMER(50)
    {
        interactor_loop();
    }
    INTERVAL_TIMER(100)
    {
        interactorAction = interactor_get();
        serialLoop();
        interactorMenu();
        // TEMPCHNG
        // if(!wifi_loop()){
        //     displayInfoAdd("discn",4);
        //     beep_music("E2E2E2E0");
        // }
    }
    tabata_loop();
    webserver_loop();

    if(onBootFlag) onBootFlag = false;
}
void checkBattery(){
    int battery_level = getBattery();
    if(onBootFlag && battery_level<99){
        displayInfoAdd("bat"+String(getBattery()), 1);
    }
    if(battery_level < 30){
        displayInfoAdd("loBat",1);
        DBG_LOGW("Battery Low!");
        DBG_LOGI(battery_level);
    }
    if(battery_level < 10){
        display_text("loBat");
        DBG_LOGW("Battery Empty!");
        delay(1000);
        deviceSleep();
    }
}

bool displayInfo(){
    if (screen.infoCounter)
    {
        display_text(screen.messages[screen.infoCounter - 1]);
        if (millis() - screen.infoMillis > 1000)
        {
            if(!onBootFlag)screen.infoCounter--;
            screen.infoMillis = millis();
        }
        return 1;
    }
    return 0;
}

TimerPhase dummy= DONE;
void indicateByPhase(TimerCurrent t_timer){
    if(lastPhase != t_timer.Phase){
        // Execute on phase change
        indicateByPhaseOnce=true;
        beep_wait(0);
    }

    // Flag for updating on ticks
    if(lastElapsed != t_timer.countDown){
        lastElapsed = t_timer.countDown;
        indicateByTick = true;
    }else{
        indicateByTick = false;
    }

    // Update by phase
    switch (t_timer.Phase)
    {
    case COUNTDOWN:
        if(indicateByPhaseOnce){
            beep_music("C1");
        }
        break;
    case BEGIN:
        if(indicateByPhaseOnce){
            beep_music("C2D2E2F5");
        }
        break;
    case REST:
        if(indicateByPhaseOnce){
            beep_music("E2D2E2D2");
        }
        break;
    case RECOVERY:
        if(indicateByPhaseOnce){
            beep_music("F2D2E2C5");
        }
        break;
    case DONE:
        if(indicateByPhaseOnce){
            beep_music("C2C2C5");
        }
        break;
    }
    // Update everytime for all phase
    uint8_t indexByPhase = static_cast<int>(t_timer.Phase);
    display_bar(t_timer.countDown, colorByPhase[indexByPhase], true, t_timer.countTime);
    
    if(t_timer.countDown < 8 && indicateByTick){
        beep(1, 3654);
        if(t_timer.countDown < 4)  beep(1,3255);
        if(t_timer.countDown == 1)  beep(5, 3255);
    }
    // Update once for all phase
    if(indicateByPhaseOnce)
    {
        // beep(beepByPhase[indexByPhase]);
        displayInfoAdd(messageByPhase[indexByPhase],2);
        DBG_LOGI(messageByPhase[indexByPhase]);
    }
    if(indicateByPhaseOnce)
    {
        indicateByPhaseOnce=false;
        lastPhase = t_timer.Phase;
    }
}

void displayMain(TimerCurrent t_timer)
{
    if(!displayInfo())
    {   
        uint8_t indexByPhase = static_cast<int>(t_timer.Phase);
        
        // Update  by phase
        indicateByPhase(t_timer);      

        if(t_timer.timerRunning){
            // Timer Running
            if(!indicateByPauseOnce){
                indicateByPauseOnce = true;
                beep_music("E1G1");
            }
            display_time(t_timer.countDown, (t_timer.countDown<6) ? true:false);
            if(tabata.preset)
                displayInfoAdd("Pre" + String(tabata.preset),1);  
        }else if(!t_timer.timerPaused){
            // Timer Idle
            // displayInfoAdd("  -  ", 2);
            display_text("  -  ");
            checkBattery();

        }else if(t_timer.timerPaused && indicateByPauseOnce){
            // Timer Paused
            indicateByPauseOnce = false;
            displayInfoAdd("PAUSED",1);
            beep_music("E1E1");
        }
    }
}

void displayInfoAdd(String t_message, uint8_t t_index)
{
    if (!(t_message == screen.lastMessage[t_index]))
    {
        DBG_LOGV(t_message);
        // beep(1);
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
        int16_t variable_multiplier[]={1,60,3600};
        variable += variable_multiplier[variable_pos] * processPos();
        if(variable < 0) variable = 0;

        if (interactorAction == PRESS){
            variable_pos++;
        }

        if (interactorAction == LONGPRESS || variable_pos==3)
        {
            if(variable > 0){
                sequenceStop();
                playTimer(0, variable);
                displayInfoAdd("START",0);
                menuState = HOME;
            }
            else
            {
                displayInfoAdd("Error",0);
            }
            variable_pos=0;
        }
        if (interactorAction == PRESSPRESS)
        {
            displayInfoAdd("EXIT",0);
            menuState = HOME;
        }

        displayInfoAdd("TIMER",0);
        interactorOutput = {String(variable), variable_pos};

        if(!displayInfo())
            display_time(variable,(interactorAction==NONE)?(variable_pos+2):0);
        display_bar((variable_flag ? 6:2), B100, true);
        break;
    }

    if (interactorAction != NONE)
        interactorAction = NONE;

    DBG_LOGV(interactorOutput.position);
    DBG_LOGV(interactorOutput.buffer);
    // return output_buffer;
}

void boot_intro(){
    display_bootanim();
    String message = "  URBAN AAKHADA";
    uint8_t scroll_max = message.length();
    for(uint8_t i=0; i<scroll_max; i++){
        display_text(message.substring(i));
        delay(300);
    }
}

// Function to simulate parts of code through serial
uint16_t beepFreq = 1200;
float beepMult = 1;
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
            beep_toggle();
            break;
        case 'Q':
            ESP.reset();
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
        case 'z':
            deviceSleep();
            break;
        case 'U':
            playTimer();
        break;
        case 'u':
            playTimer(-1);
        break;
        case 'V':
            beepFreq += 10;
            Serial.println(beepFreq);
        break;
        case 'v':
            beepFreq -= 10;
            Serial.println(beepFreq);
        break;
        case 'C':
            beep_music("E1E1F1G1G1F1E1D1C1C1D1E1E1D1D1E1E1F1G1G1F1E1D1C1C1D1E1D1E1F1F1E1E1F1CE1D1C1C1D1E1E1D1D1E1E1F1G1G1F1E1D1C1C1D1E1D1E1F1F1E1E1F1E1D1C1C1D1E1D1E1");
        break;
        case 'c':
            beep_music("B1A1G1A1B1B1B2A1A1A2B1B1B2B1A1G1A1B1B1B2A1A1B1A1G2G2");
        break;
        case 'B':
            beep_music("E1E1E2E1E1E2E1G1C1D1E4F1F1F1F1F1E1E1E1E1D1D1E1D2G2");
        break;
        case 'b':
            beep_music("C1C1G1G1A1A1G2F1F1E1E1D1D1C2");
        break;
        case 'g':
            beep_music("C1C1D1C1F1E1C1C1D1C1G1F1C1C1C2A1A1G1F1E1D1B1B1A1F1E1D1C2");
        break;
        case ',':
            display_bootanim();
        break;
        case '.':
            boot_intro();
        break;
        }
        Serial.flush();
    }
}
