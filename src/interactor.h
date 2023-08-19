#include "Arduino.h"
#include "board_defs.h"
#include <RotaryEncoder.h>

RotaryEncoder encoder(ENC_SCL_PIN, ENC_SDA_PIN, ENC_BTN_PIN);
int8_t buzz_state = 0;
bool beep_enable = true;
uint8_t buzz_intensity = 255;
uint8_t press_counter = 0;
unsigned long press_counter_millis = 0;

struct InteractorOutput
{
    String buffer;
    int16_t position;
} interactorOutput;

enum InteractorAction
{
    NONE,
    PRESS,
    PRESSPRESS,
    PRESSPRESSPRESS,
    LONGPRESS,
    LONGLONGPRESS,
    UP,
    DOWN
};

void IRAM_ATTR encoderISR() // interrupt service routines need to be in ram
{
    encoder.readAB();
}

void IRAM_ATTR encoderButtonISR()
{
    encoder.readPushButton();
}

#define MAX_BEEP_BUFFER 50
struct BeepBuffer{
    uint8_t ticks;
    uint16_t intensity;
}beep_buffer[MAX_BEEP_BUFFER];


uint8_t beep_buffer_counter = 0;
uint8_t beep_execute_counter = 0;
bool beep_wait_flag = false;
void beep_wait(bool t_state=1){
    if(t_state == 0)
        beep_wait_flag = false;
}

void beep_toggle(){
    beep_enable = !beep_enable;
    noTone(BUZZ_PIN);
}
void beep(uint8_t t_state = 1, uint16_t t_intensity = 1)
{  
    if(beep_enable){
        if(!beep_wait_flag){
            beep_buffer[beep_buffer_counter] = {t_state, t_intensity};
            beep_buffer_counter++;
            if(beep_buffer_counter == MAX_BEEP_BUFFER)
                beep_buffer_counter = MAX_BEEP_BUFFER-1;
        }
        if(t_intensity == 0){
            beep_wait_flag = true;
        }
    }

}

// uint16_t beep_notes[]={
//     3600,//A
//     3650,//B
//     850,//C
//     1240,//D
//     1850,//E
//     2500,//F
//     2700//G
// }
uint16_t beep_notes[]={
    4058,//A
    4562,//B
    2441,//C
    2741,//D
    3048,//E
    3255,//F
    3654 //G
};
void beep_music(String t_music){
    
    DBG_LOGV(t_music);
    for(uint8_t i=0; i<t_music.length(); i+=2){
        // Sanity check for music characters
        if(t_music[i+1] > '0'){
            // Get time integer by subtracting char '0'
            uint8_t m_time = (uint8_t)(t_music[i+1] - '0');
            // Set default frequency to 0
            uint16_t m_frequency = 0;
            if(t_music[i]>'A' && t_music[i]<'H'){
                m_frequency = beep_notes[(uint8_t)(t_music[i] - 'A')];
            }
            beep(m_time, m_frequency);
        }
    }
}

void beep_loop()
{
    DBG_LOGV(beep_buffer_counter);
    if(beep_buffer_counter > 0){    
        if (beep_buffer[beep_execute_counter].ticks > 0)
        {
            if(beep_buffer[beep_execute_counter].intensity>0){
                if(beep_buffer[beep_execute_counter].intensity == 1)
                    digitalWrite(BUZZ_PIN, 1);
                else
                    tone(BUZZ_PIN , beep_buffer[beep_execute_counter].intensity);
            }
            beep_buffer[beep_execute_counter].ticks--;
        }
        else
        {
            noTone(BUZZ_PIN);
            if(beep_execute_counter == beep_buffer_counter)
            {
                beep_buffer_counter = 0;
                beep_execute_counter = 0;
                beep_wait_flag = false;
            }
            if(beep_buffer_counter>0)
            {   
                beep_execute_counter++;
            }
        }
    }
    // Serial.print(" ");
    // Serial.println(beep_buffer_counter);
}

void encoder_init()
{
    encoder.begin();                                                                // set encoders pins as input & enable built-in pullup resistors
    attachInterrupt(digitalPinToInterrupt(ENC_SCL_PIN), encoderISR, CHANGE);        // call encoderISR()    every high->low or low->high changes
    attachInterrupt(digitalPinToInterrupt(ENC_BTN_PIN), encoderButtonISR, FALLING); // call pushButtonISR() every high->low              changes
}

unsigned long push_button_millis = 0;
unsigned long push_button_time = 0;
bool push_button_new = false;
uint16_t encoder_push_loop()
{
    if (encoder.getPushButton())
    {
        push_button_millis = millis();
        push_button_new = true;
    }
    if (digitalRead(ENC_BTN_PIN) && push_button_new)
    {
        push_button_time = (millis() - push_button_millis);
        push_button_new = false;
        return push_button_time;
    }

    return 0;
}

int16_t processPos(InteractorAction t_action)
{
    if (t_action == UP)
        return 1;
    if (t_action == DOWN)
        return -1;
    return 0;
}
int16_t processPos()
{
    int16_t position = encoder.getPosition();
    encoder.setPosition(0);
    if(position>0)
    return position*position;
    else 
    return -(position*position);
}

void interactor_init()
{
    encoder_init();
    pinMode(BUZZ_PIN, OUTPUT);
    pinMode(D8, OUTPUT);
}

void interactor_loop()
{
    if(beep_enable)
        beep_loop();
}



InteractorAction interactor_get()
{
    uint16_t push_time = encoder_push_loop();
    if (push_time > 5000)
    {
        beep(3);
        PRT_LOGD("LONGLONGPRESS");
        return LONGLONGPRESS;
    }
    else if (push_time > 1000)
    {
        beep(2);
        PRT_LOGD("LONGPRESS");
        return LONGPRESS;
    }
    else if (push_time > 0)
    {
        // beep(1);
        press_counter++;
        press_counter_millis = millis();
        PRT_LOGD("PRESS");
        return NONE;
    }
    if(press_counter>0 && (millis()-press_counter_millis>500)){
        press_counter_millis = millis();
        PRT_LOGD(press_counter);
        // beep(press_counter);

        InteractorAction returnAction = PRESS;
        if(press_counter>2)returnAction = PRESSPRESSPRESS;
        else if(press_counter>1)returnAction = PRESSPRESS;

        press_counter=0;
        return returnAction;
    }

    int16_t position = encoder.getPosition();
    if (position > 0)
    {
        encoder.setPosition(position-1);
        return UP;
    }
    else if (position < 0)
    {
        encoder.setPosition(position+1);
        return DOWN;
    }

    return NONE;
}
