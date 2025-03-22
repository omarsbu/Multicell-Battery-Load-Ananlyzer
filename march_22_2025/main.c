#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

int main(void)
{
	adc_mode = 0x00;
	adc_value = 0;
	adc_vref = 3.3;
	battery_voltage_divider_ratios = 5;
	current_sensing_voltage_divider_ratios = 6;
	shunt_resistance_ohms = 0.00008;
	OPAMP_gain = 30;
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
//	OPAMP_Instrumentation_init();
	
	/* Initialize Fan PWM module */
	Fan_PWM_init();
	
	/* Initialize pushbutton IO pins */
	PB_init();
	
	LOCAL_INTERFACE_FSM();
	
	CPU_CCP = CCP_IOREG_gc;		// change protected IO register
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKOUT_bm;	// enable CLKOUT on PA7	
	sei(); // enable interrupts

	while(1)
	{
//	ADC_init(0x00);
//	ADC_channelSEL(OPAMP_ADC_CHANNEL, GND_ADC_CHANNEL);
//	adc_value = ADC_read();		
	
	/* Multiply by divider ratio to undo attenuation, divide by gain to undo gain, divide by resistance to convert to amps */
//	load_current_amps = ((adc_value*current_sensing_voltage_divider_ratios) / (OPAMP_gain*shunt_resistance_ohms));	
//		ADC_init(0x00);
//		ADC_channelSEL(B1_ADC_CHANNEL, GND_ADC_CHANNEL);
//		adc_value = ADC_read();
//		asm volatile("nop");

//		ADC_channelSEL(B1_ADC_CHANNEL, GND_ADC_CHANNEL);
//		adc_value = ADC_read();
		
//		load_current_amps = load_current_Read();
		asm volatile("nop");	


	}
}

