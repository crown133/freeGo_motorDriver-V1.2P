#include "adc.h"


ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

uint32_t adc_buf[CH_NUM * ADC_SAMPLE_NUM];

uint32_t adc_val[3];

/* ADC1 init function */
void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig;
    
    HAL_ADC_Stop_DMA(&hadc1);
    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = CH_NUM;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
    sConfig.Channel = ADC_CHANNEL_10;                   //adc采样时间 = 转换时间 + 读取时间  
    sConfig.Rank = 1;                                   //转换时间 = 采样时间 + 12.5 adc周期
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_12;
    sConfig.Rank = 2;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfig.Channel = ADC_CHANNEL_13;
    sConfig.Rank = 3;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_ADC_Start_DMA(&hadc1, adc_buf, CH_NUM * ADC_SAMPLE_NUM); //开启第一次DMA传送
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    if (adcHandle->Instance == ADC1)
    {

        /* ADC1 clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();

        /**ADC1 GPIO Configuration
        PC0     ------> ADC1_IN10 //母线电压
        PC2     ------> ADC1_IN12
        PC3     ------> ADC1_IN13    
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        
        /* ADC1 DMA Init */
        __HAL_RCC_DMA2_CLK_ENABLE();        //f4 无adc校准

        hdma_adc1.Instance = DMA2_Stream0;
        hdma_adc1.Init.Channel = DMA_CHANNEL_0;
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_adc1.Init.Mode = DMA_CIRCULAR;
        hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
        {
            Error_Handler();
        }
        
        HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);  //cube 默认开启dma中断，会调用回调函数，浪费时间
        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{

    if (adcHandle->Instance == ADC1)
    {

        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

        /**ADC1 GPIO Configuration
        PC0     ------> ADC1_IN10 //母线电压
        PC2     ------> ADC1_IN12
        PC3     ------> ADC1_IN13    
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);

        /* ADC1 DMA DeInit */
        HAL_DMA_DeInit(adcHandle->DMA_Handle);

        /* ADC1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(ADC_IRQn);
    }
}

//ADC均值滑动滤波
void ADC_Filter(ControllerStruct* controller)
{
    uint16_t i;
    uint32_t sum[CH_NUM] = {0};

    for (i = 0; i < ADC_SAMPLE_NUM; i++)
    {
        sum[0] += adc_buf[CH_NUM * i + 0];
        sum[1] += adc_buf[CH_NUM * i + 1];
        sum[2] += adc_buf[CH_NUM * i + 2];
    }
    controller->adc3_raw = sum[0] / ADC_SAMPLE_NUM; //Voltage
    controller->adc2_raw = sum[1] / ADC_SAMPLE_NUM; //phase A
    controller->adc1_raw = sum[2] / ADC_SAMPLE_NUM; //phase B
}


