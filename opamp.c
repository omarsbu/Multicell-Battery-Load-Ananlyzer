/*
 * opamp.c
 *
 * Created: 12/18/2024 4:01:59 PM
 *  Author: Omar
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

void OPAMP_Instrumentation_init(void)
{
	// DBGRUN disabled;
	OPAMP.DBGCTRL = 0x0;
	// Rail-to-Rail voltage ranges;
	OPAMP.PWRCTRL = 0x00;
	// OP0 - Input Configuration
	OPAMP.OP0INMUX = (OPAMP_OP0INMUX_MUXNEG_OUT_gc | OPAMP_OP0INMUX_MUXPOS_INP_gc);

	// OP0 - Resistor Ladder Configuration, default gain is 15
	switch(OPAMP_gain)
	{
		case 15:		
			OPAMP.OP0RESMUX = (OPAMP_OP0RESMUX_MUXBOT_GND_gc |
			OPAMP_OP0RESMUX_MUXTOP_OUT_gc | OPAMP_OP0RESMUX_MUXWIP_WIP0_gc);			
			break;
			
		case 7:
			OPAMP.OP0RESMUX = (OPAMP_OP0RESMUX_MUXBOT_GND_gc |
			OPAMP_OP0RESMUX_MUXTOP_OUT_gc | OPAMP_OP0RESMUX_MUXWIP_WIP1_gc);
			break;
			
		case 3:
			OPAMP.OP0RESMUX = (OPAMP_OP0RESMUX_MUXBOT_GND_gc |
			OPAMP_OP0RESMUX_MUXTOP_OUT_gc | OPAMP_OP0RESMUX_MUXWIP_WIP2_gc);
			break;	
			
		case 1:
			OPAMP.OP0RESMUX = (OPAMP_OP0RESMUX_MUXBOT_GND_gc |
			OPAMP_OP0RESMUX_MUXTOP_OUT_gc | OPAMP_OP0RESMUX_MUXWIP_WIP3_gc);
			break;
				
		default: 
			OPAMP.OP0RESMUX = (OPAMP_OP0RESMUX_MUXBOT_GND_gc |
			OPAMP_OP0RESMUX_MUXTOP_OUT_gc | OPAMP_OP0RESMUX_MUXWIP_WIP0_gc);
	}

	// OP1 - Input Configuration
	OPAMP.OP1INMUX = (OPAMP_OP1INMUX_MUXNEG_OUT_gc | OPAMP_OP1INMUX_MUXPOS_INP_gc);

	// OP1 - Resistor Ladder Configuration
	OPAMP.OP1RESMUX = (OPAMP_OP1RESMUX_MUXBOT_OFF_gc |
	OPAMP_OP1RESMUX_MUXTOP_OFF_gc | OPAMP_OP1RESMUX_MUXWIP_WIP0_gc);

	// OP2 - Input Configuration
	OPAMP.OP2INMUX = (OPAMP_OP2INMUX_MUXNEG_WIP_gc |
	OPAMP_OP2INMUX_MUXPOS_LINKWIP_gc);

	// OP2 - Resistor Ladder Configuration, default gain is 15
	switch(OPAMP_gain)
	{
		case 15:
		OPAMP.OP2RESMUX = (OPAMP_OP2RESMUX_MUXBOT_LINKOUT_gc |
		OPAMP_OP2RESMUX_MUXTOP_OUT_gc | OPAMP_OP2RESMUX_MUXWIP_WIP7_gc);
		break;
		
		case 7:
		OPAMP.OP2RESMUX = (OPAMP_OP2RESMUX_MUXBOT_LINKOUT_gc |
		OPAMP_OP2RESMUX_MUXTOP_OUT_gc | OPAMP_OP2RESMUX_MUXWIP_WIP6_gc);
		break;
		
		case 3:
		OPAMP.OP2RESMUX = (OPAMP_OP2RESMUX_MUXBOT_LINKOUT_gc |
		OPAMP_OP2RESMUX_MUXTOP_OUT_gc | OPAMP_OP2RESMUX_MUXWIP_WIP5_gc);		
		break;
		
		case 1:
		OPAMP.OP2RESMUX = (OPAMP_OP2RESMUX_MUXBOT_LINKOUT_gc |
		OPAMP_OP2RESMUX_MUXTOP_OUT_gc | OPAMP_OP2RESMUX_MUXWIP_WIP3_gc);
		break;
		
		default:
		OPAMP.OP2RESMUX = (OPAMP_OP2RESMUX_MUXBOT_LINKOUT_gc |
		OPAMP_OP2RESMUX_MUXTOP_OUT_gc | OPAMP_OP2RESMUX_MUXWIP_WIP7_gc);
	}	

	//ALWAYSON enabled; EVENTEN disabled; OUTMODE Output Driver in Normal Mode; RUNSTBY enabled;
	OPAMP.OP0CTRLA = 0x85;
	OPAMP.OP1CTRLA = 0x85;
	OPAMP.OP2CTRLA = 0x85;

	// SETTLE 127;
	OPAMP.OP0SETTLE = 0x7F;
	OPAMP.OP1SETTLE = 0x7F;
	OPAMP.OP2SETTLE = 0x7F;

	// Enable
	OPAMP.CTRLA |= OPAMP_ENABLE_bm;
}

float load_current_Read(void)
{
	/* Select OP2 output as ADC input */
	ADC0.MUXPOS = 0x0A; // ADC input pin AIN10, OPAMP OP2 output
	ADC0.MUXNEG = 0x40; // GND

	/* Perform ADC conversion */
	ADC_startConversion();
	while(ADC_isConversionDone() != 0x01);	// wait for conversion to finish
	ADC_stopConversion();
	adc_value = ADC_read();
	adc_value = adc_value >> 4;//the 12 bit result was left adjusted, so shift right 4 places to fix
	
	/* Convert ADC voltage to current, only accurate if load current is between 100A - 500A */
	/* Measure instrumentation amplifier gain versus output voltage and store in array, start
	   at largest gain in array and loop back to lowest gain, comparing the value read on each 
	   iteration. The entry in the array will correspond to a incremental gain of some
	   value that is not determined yet.
	   
		for (uint8_t i = 10; i >= 0; i--)
		{
			if ((adc_vref*(adc_value/2048)) >= OPAMP_voltage_versus_gain[i])
			{
				gain = minimum_OPAMP_gain + i*OPAMP_gain_increment;
				break;
			}
		}
	*/	

	uint8_t gain = 15;	// default value, change later
	load_current = current_sensing_voltage_divider_ratios*((float)((((float)adc_value / 2048) * adc_vref)/(gain*shunt_resistance_ohms)));
	
	return load_current;
}

