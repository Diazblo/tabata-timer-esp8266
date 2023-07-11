#include "Arduino.h"
#include "board_defs.h"
#include <RotaryEncoder.h>

RotaryEncoder encoder(ENC_SCL_PIN, ENC_SDA_PIN, ENC_BTN_PIN);
uint8_t buzz_state = 0;

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

void beep(uint8_t t_state = 1, uint16_t t_delay = 0)
{
    buzz_state += t_state;
}
void beep_loop()
{
    if (buzz_state > 0)
    {
        digitalWrite(BUZZ_PIN, 1);
        buzz_state--;
    }
    else
    {
        digitalWrite(BUZZ_PIN, 0);
    }
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
        push_button_time = (millis() - push_button_millis) / 10;
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

void interactor_init()
{
    encoder_init();
    pinMode(BUZZ_PIN, OUTPUT);
}

void interactor_loop()
{
    beep_loop();
}

InteractorAction interactor_get()
{
    uint16_t push_time = encoder_push_loop();
    if (push_time > 200)
    {
        beep(3);
        Serial.println("LONGLONGPRESS");
        return LONGLONGPRESS;
    }
    else if (push_time > 70)
    {
        beep(2);
        Serial.println("LONGPRESS");
        return LONGPRESS;
    }
    else if (push_time > 0)
    {
        beep(1);
        Serial.println("PRESS");
        return PRESS;
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
