/* Includes ------------------------------------------------------------------*/
#include "sys.h"

#include "spi.h"
#include "tim.h"
#include "led.h"
#include "delay.h"
#include "adc.h"

#include "TLE5012B.h"
#include "position.h"
#include "DRV8323.h"

#include "FastMath.h"
#include "foc.h"
#include "flash.h"
#include "motor_config.h"
#include "hw_config.h"
#include "user_config.h"
#include "current_controller_config.h"

////////// state machine ///////////
#define REST_MODE 0
#define CALIBRATION_MODE 1
#define MOTOR_MODE 2
#define SETUP_MODE 4
#define ENCODER_MODE 5

u8 state = 8;  //state
volatile u8 state_change;

///////////////////////
float __float_reg[64]; // Floats stored in flash
int __int_reg[256];    // Ints stored in flash.  Includes position sensor calibration lookup table


u8 state_flag = 0;


int main(void)
{
    HAL_Init(); //Reset of all peripherals, Initializes the Flash interface and the Systick.

    SystemClock_Config();
    delay_init(168);

    /* Initialize all configured peripherals */
    led_Init();
    led_on;
    delay_us(100);

    SysNVIC_SetPriority();

    TLE5012B_init(); //Positon Sensor Init
    delay_us(100);

    MX_ADC1_Init(); //ADC
    delay_us(100);

    MX_TIM1_Init(); //PWM output
    delay_us(100);

    reset_foc(&controller);    // Reset current controller
    reset_observer(&observer); // Reset observer
    delay_us(100);

    drv8323_init(); //MOS Driver Init
    delay_us(100);

		flash_load();
		delay_us(100);
				
    MX_TIM3_Init(); //40kHz timer global interrupt
    HAL_TIM_Base_Start_IT(&htim3);
    delay_ms(500);

//    drv_enable_gd();
		
    while (1)
    {
			
        //        delay_ms(10);
//								float theta_ref;
//								const int n = 128 * NPP;               // number of positions to be sampled per mechanical rotation.  Multiple of NPP for filtering reasons (see later)
//								const int n2 = 40;                     // increments between saved samples (for smoothing motion)
//								float delta = 2 * PI * NPP / (n * n2); // change in angle between samples
//								float v_d = 0.15f;                     // Put volts on the D-Axis
//								float v_q = 0.0f;
//								float v_u, v_v, v_w = 0;
//								float dtc_u, dtc_v, dtc_w = .5f;
        // drv_enable_gd();
        //  while(state_flag--)
        //  {
        //              led_on;
        //      for (int i = 0; i < n; i++)
        //      { // rotate forwards
        //          for (int j = 0; j < n2; j++)
        //          {
        //              theta_ref += delta;
        //              abc(theta_ref, v_d, v_q, &v_u, &v_v, &v_w);      // inverse dq0 transform on voltages
        //              svm(1.0, v_u, v_v, v_w, &dtc_u, &dtc_v, &dtc_w); // space vector modulation
        //              TIM1->CCR3 = (PWM_ARR >> 1) * (1.0f - dtc_u);   // Check phase ordering
        //              TIM1->CCR2 = (PWM_ARR >> 1) * (1.0f - dtc_v);   // Set duty cycles
        //              TIM1->CCR1 = (PWM_ARR >> 1) * (1.0f - dtc_w);
        //              delay_us(100);
        //          }
        //      }
        //      led_off;
        //      delay_ms(50);

        //  }
    }
}

//Control_Loop which runs in 40KHz
void TIM3_IRQHandler(void)
{
    PS_Value(DT);
    ADC_Filter(&controller);
    controller.v_bus = 0.95f * controller.v_bus + 0.05f * V_SCALE * (float)controller.adc3_raw;

    controller.theta_mech = (1.0f / GR) * PS.MechPosition;
    controller.theta_elec = PS.ElecPosition;
    controller.dtheta_mech = (1.0f / GR) * PS.MechVelocity;
    controller.dtheta_elec = PS.ElecVelocity;

/// Check state machine state, and run the appropriate function ///
//    switch (state)
//    {
//    case REST_MODE: // Do nothing
//        if (state_change)
//        {
//            enter_menu_state();
//        }
//        break;

//    case CALIBRATION_MODE: // Run encoder calibration procedure
//        if (state_change)
//        {
//            calibrate();
//        }
//        break;

//    case MOTOR_MODE: // Run torque control
//        if (state_change)
//        {
//            enter_torque_mode();
//            count = 0;
//        }
//        else
//        {
//            /*
//                if(controller.v_bus>28.0f){         //Turn of gate drive if bus voltage is too high, to prevent FETsplosion if the bus is cut during regen
//                    gpio.
//                    ->write(0);
//                    controller.ovp_flag = 1;
//                    state = REST_MODE;
//                    state_change = 1;
//                    printf("OVP Triggered!\n\r");
//                    }
//                    */

//            if ((controller.timeout > CAN_TIMEOUT) && (CAN_TIMEOUT > 0))
//            {
//                controller.i_d_ref = 0;
//                controller.i_q_ref = 0;
//                controller.kp = 0;
//                controller.kd = 0;
//                controller.t_ff = 0;
//            }

//            torque_control(&controller);
//            commutate(&controller, &observer, &gpio, controller.theta_elec); // Run current loop

//            controller.timeout++;
//            count++;
//        }
//        break;
//    case SETUP_MODE:
//        if (state_change)
//        {
//            enter_setup_state();
//        }
//        break;
//    case ENCODER_MODE:
//        print_encoder();
//        break;
//    }
    if (state_flag)
    {
				__float_reg[2] = state;
				delay_ms(10);
				flash_write();
			  delay_ms(10);
				state_flag = 0;
//        commutate(&controller, &observer, controller.theta_elec);
    }

    HAL_TIM_IRQHandler(&htim3);
}
