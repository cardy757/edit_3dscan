#ifndef FSSTEPPER_H
#define FSSTEPPER_H

//#include "staticHeaders.h"
#include "serial.h"
#include "geometries.h"

#define MC_TURN_LASER_OFF      200
#define MC_TURN_LASER_ON       201
#define MC_PERFORM_STEP        202
#define MC_SET_DIRECTION_CW    203
#define MC_SET_DIRECTION_CCW   204
#define MC_TURN_STEPPER_ON     205
#define MC_TURN_STEPPER_OFF    206
#define MC_TURN_LIGHT_ON       207
#define MC_TURN_LIGHT_OFF      208
#define MC_ROTATE_LASER        209
#define MC_FABSCAN_PING        210
#define MC_FABSCAN_PONG        211
#define MC_SELECT_STEPPER      212
#define MC_LASER_STEPPER       11
#define MC_TURNTABLE_STEPPER   10


/********************************/
/*          FS_MISC             */
/********************************/

typedef enum
{
  FS_DIRECTION_CCW,
  FS_DIRECTION_CW
} FSDirection;

class FSTurntable
{
private:
    double direction; //in which direction is the table turning, left or right
    GlobalPoint rotation; //the current rotation of the turntable

public:
    double degreesPerStep; //the stepper performs step of a certain number of degrees

    FSTurntable();
    void turnNumberOfSteps(unsigned int steps); //tell turntable to move a certain number of steps
    void turnNumberOfDegrees(double degrees);   //tell turntable to move a certain number of degrees
    void setDirection(FSDirection direction);   //set the direction of the turntable, either clockwise or counterclock wise
    void toggleDirection();                     //change the direction
    void setRotation(GlobalPoint r);                //set the current rotation of the turntable
    GlobalPoint getRotation(void);                  //get the current rotation of the turntable
    void selectStepper();
    void enable(void);                          //enable the turntable (enable the stepper engine)
    void disable(void);                         //disable the turntable (disable the stepper engine)
};

#endif // FSSTEPPER_H
