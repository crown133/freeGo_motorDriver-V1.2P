#ifndef __FOC_H
#define __FOC_H

#include "sys.h"
#include "structs.h"
#include "FastMath.h"
#include "hw_config.h"
#include "current_controller_config.h"
#include "arm_math.h"

#include "adc.h"
#include "tim.h"
#include "delay.h"

extern ControllerStruct controller;
extern ObserverStruct observer;


void abc( float theta, float d, float q, float *a, float *b, float *c);
void svm(float v_bus, float u, float v, float w, float *dtc_u, float *dtc_v, float *dtc_w);

void zero_current(int *offset_1, int *offset_2); 
void reset_foc(ControllerStruct *controller);
void reset_observer(ObserverStruct *observer);

void commutate(ControllerStruct *controller, ObserverStruct *observer, float theta);


#endif
