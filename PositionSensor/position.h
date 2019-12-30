#ifndef __POSITION_H
#define __POSITION_H


typedef struct
{
    float oldAngle;
    float position;
    float oldPosition;

    float MechPosition;
    float MechOffset;
    float ElecPosition;
    float ElecOffset;

    int rotations;

    float MechVelocity;
    float ElecVelocity;
    float ElecVeloFilt;
    float velVec[40];

} PositionSensor;

extern PositionSensor PS;
void PS_Value(float dt); //calculate position & velocity

#endif
