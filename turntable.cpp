#include "turntable.h"
#include "edit_3dscan.h"

Turntable::Turntable()
{
    reset();
}

void Turntable::reset()
{
    degreesPerStep = 360.0f/200.0f/16.0f; //the size of a microstep
    direction = FS_DIRECTION_CW;
    rotation = MakeGlobalPoint(0.0f, 0.0f, 0.0f);
}

void Turntable::turnNumberOfSteps(unsigned int steps)
{
    unsigned char size = ((steps - 1) / 256 + 1) * 2;
    char *c = new char[size + 1];
    unsigned int s = steps;
    for (unsigned int i = 0; i < size / 2; i++)
    {
        c[2 * i] = MC_PERFORM_STEP;
        if( s < 256)
        {
            c[2 * i + 1] = s % 256;
        }
        else
        {
            c[2 * i + 1] = 255;
            s -= 255;
        }
    }
    c[size] = 0;
    Edit3DScanPlugin::arduino->writeChars(c);
    delete [] c;
}

void Turntable::turnNumberOfDegrees(double degrees)
{
    int steps = (int)(degrees/degreesPerStep);
    if (direction==FS_DIRECTION_CW)
    {
      rotation.y -= degrees;
    }
    else if (direction==FS_DIRECTION_CCW)
    {
      rotation.y += degrees;
    }
    turnNumberOfSteps(steps);
}

void Turntable::setDirection(FSDirection d)
{
    direction = d;
    char c = (d == FS_DIRECTION_CW)? MC_SET_DIRECTION_CW : MC_SET_DIRECTION_CCW;
    Edit3DScanPlugin::arduino->writeChar(c);
}

void Turntable::toggleDirection()
{
    FSDirection d = (direction == FS_DIRECTION_CW)?FS_DIRECTION_CCW:FS_DIRECTION_CW;
    setDirection(d);
}

void Turntable::enable(void)
{
    reset();
    Edit3DScanPlugin::arduino->writeChar(MC_TURN_STEPPER_ON);
}

void Turntable::disable(void)
{
    Edit3DScanPlugin::arduino->writeChar(MC_TURN_STEPPER_OFF);
}

GlobalPoint Turntable::getRotation()
{
    return rotation;
}
