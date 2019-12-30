#ifndef __ADC_H
#define __ADC_H

#include "sys.h"
#include "structs.h"


#define CH_NUM         3
#define ADC_SAMPLE_NUM 5

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern uint32_t adc_buf[CH_NUM * ADC_SAMPLE_NUM];
extern uint32_t adc_val[3];

void MX_ADC1_Init(void);
void ADC_Filter(ControllerStruct* controller);


#endif

