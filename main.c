#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

int main(void)
{
	adc_mode = 0x00;
	adc_value = 0;
	adc_vref = 5;
	battery_voltage_divider_ratios = 4;
	current_sensing_voltage_divider_ratios = 6;
	shunt_resistance_ohms = 0.0008;
	OPAMP_gain = 15;
	cursor = 1;
	quad_pack_entry = 0;

	LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
	TEST_CURRENT_STATE = ERROR;
	VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
	PB_PRESS = NONE;
	
	/* Initialize LCD */
	init_lcd();
	
	/* Initialize ADC */
	ADC_init(0x00);
	
	/* Initialize  Instrumentation Amplifier */
	OPAMP_Instrumentation_init();
	
	/* Initialize Fan PWM module */
	Fan_PWM_init();
	
	/* Initialize pushbutton IO pins */
	PB_init();

	sei(); // enable interrupts

	while(1)
	{
		asm volatile("nop");
	}
}

