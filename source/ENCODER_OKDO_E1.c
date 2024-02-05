#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC55S69_cm33_core0.h"
#include "fsl_debug_console.h"

#include "oled.h"

#define readCHB (GPIO_PinRead(ENCODER_GPIO_CHB_GPIO_GPIO , ENCODER_GPIO_CHB_GPIO_PORT, ENCODER_GPIO_CHB_GPIO_PIN))
#define I2C_OLED ((I2C_Type *)I2C1_BASE)
#define PWM_Count 4

char sbuff[32];
uint8_t pwm[PWM_Count];
uint8_t selected = 0;
bool mode = false;
bool refresh = false;

uint8_t firts = 0;

void CB_ENC_CLK(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	if(readCHB)
	{
		if(mode == false)
		{
			selected++;
		}
		else
		{
			pwm[selected]++;
		}
	}
	else
	{
		if(mode == false)
		{
			selected--;
		}
		else
		{
			pwm[selected]--;
		}
	}

	if(selected < 0)
	{
		selected = (PWM_Count - 1);
	}

	if(selected > (PWM_Count - 1))
	{
		selected = 0;
	}

	if(pwm[selected] > 100)
	{
		pwm[selected] = 100;
	}

	if(pwm[selected] < 0)
	{
		pwm[selected] = 0;
	}

	refresh = true;

}

void CB_ENC_SW(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	mode = !mode;
	refresh = true;
}

void pwmUpdate()
{
	CTIMER_UpdatePwmDutycycle(CTIMER2_PERIPHERAL, CTIMER2_PWM_PERIOD_CH, CTIMER2_PWM_1_CHANNEL, 100 - pwm[0]);
	CTIMER_UpdatePwmDutycycle(CTIMER2_PERIPHERAL, CTIMER2_PWM_PERIOD_CH, CTIMER2_PWM_2_CHANNEL, 100 - pwm[1]);
}

void screenUpdate()
{
	OLED_Clear_Screen(0);

	OLED_Puts(19, 0, "[PWM GENERATOR]");
	OLED_Draw_Line(0, 12, 127, 12);

	for(int i = 0; i<PWM_Count; i++)
	{

	    if(selected == i)
	    {
	    	if(mode)
	    	{
	    		sprintf(sbuff,"[PWM %d:]       [%03d]", i, pwm[i]);
	    	}
	    	else
	    	{
	    		sprintf(sbuff,"[PWM %d:]        %03d ", i, pwm[i]);
	    	}
	    }
	    else
	    {
	    	sprintf(sbuff," PWM %d:         %03d ", i, pwm[i]);
	    }

	    OLED_Puts(0, 2 + i, sbuff);
	}

	OLED_Refresh_Gram();
}

int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    PRINTF("Hello World \r\n");

    /* Initialize OLED */
    OLED_Init(I2C_OLED);

	refresh = true;

    while(1)
    {
    	if(refresh)
    	{
    		screenUpdate();
        	pwmUpdate();
        	refresh = false;
    	}
    }
    return 0 ;
}
