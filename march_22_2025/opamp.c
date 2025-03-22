#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

//***************************************************************************
//
// Function Name : "OPAMP_Instrumentation_init"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function configures the 3 internal op amps as an instrumentation
//  amplifier with a gain defined in the main file
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
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
//***************************************************************************
//
// Function Name : "OPAMP_Instrumentation_init"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function reads output of the instrumentation amplifier and converts
//	it to a current based on the shunt resistance value.
//
// Inputs : none
//
// Outputs : none
//		float load_current: Analog Load current through shunt in amps
//
//**************************************************************************
float load_current_Read(void)
{	
	/* Put ADC in single-ended mode and select OP2 output */
	ADC_init(0x00);
	ADC_channelSEL(OPAMP_ADC_CHANNEL, GND_ADC_CHANNEL);
	adc_value = ADC_read();		
	
	/* Multiply by divider ratio to undo attenuation, divide by gain to undo gain, divide by resistance to convert to amps */
	load_current_amps = ((adc_value*current_sensing_voltage_divider_ratios) / (OPAMP_gain*shunt_resistance_ohms));	
	
	if(load_current_amps < 10)
		return 0;
	else
		return load_current_amps;
}
