#include"Arduino.h"

struct InteractorOutput{
    String buffer;
    uint16_t position;
}interactorOutput;

enum InteractorState
{
    HOME,
    SETANDSTART
} interactorState;

enum InteractorAction
{
    NONE,
    PRESS,
    PRESSPRESS,
    LONGPRESS,
    LONGLONGPRESS,
    UP,
    DOWN
} interactorAction;

int16_t processPos(InteractorAction t_action)
{
    if (t_action == UP)
        return 1;
    if (t_action == DOWN)
        return -1;
    return 0;
}

