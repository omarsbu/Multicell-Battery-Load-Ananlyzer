/*
 * adc.c
 *
 * Created: 12/18/2024 4:01:07 PM
 *  Author: tyler
 */ 

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

//***************************************************************************
//
// Function Name : "ADC_init"
// Target MCU : AVR128DB48
// DESCRIPTION
// Initializes the ADC0 module of the AVR128DB48 for differential
// mode, VDD reference, 12-bit resolution, free-run mode, 16
// sample accumulation, and clock prescalar divided by 4.
// Enables interrupts and ADC0 module.
//
// Inputs : None
//
// Outputs : None
//
//
//**************************************************************************
void ADC_init(void)
{
	// Use VDD as reference
	VREF.ADC0REF = VREF_REFSEL_VDD_gc;
	
	// 12-bit resolution, Free-Run mode, Differential mode, Right adjusted, Enable
	ADC0.CTRLA = (ADC_RESSEL_12BIT_gc | ADC_FREERUN_bm | ADC_CONVMODE_bm | ADC_ENABLE_bm);

	//enables interrupt
	ADC0.INTCTRL |= ADC_RESRDY_bm;

	// Set to accumulate 16 samples
	ADC0.CTRLB = ADC_SAMPNUM_ACC16_gc;
	
	// Divided CLK_PER by 4
	ADC0.CTRLC = ADC_PRESC_DIV4_gc;
	
}

//***************************************************************************
//
// Function Name : "ADC_startConversion"
// Target MCU : AVR128DB48
// DESCRIPTION
// Starts a conversion on ADC0.
//
// Inputs : None
//
// Outputs : None
//
//
//**************************************************************************
void ADC_startConversion(void)
{
	ADC0.COMMAND = ADC_STCONV_bm;// starts ADC conversion
}

//***************************************************************************
//
// Function Name : "ADC_stopConversion"
// Target MCU : AVR128DB48
// DESCRIPTION
// Stops a conversion on ADC0.
//
// Inputs : None
//
// Outputs : None
//
//
//**************************************************************************
void ADC_stopConversion(void)
{
	ADC0.COMMAND = ADC_SPCONV_bm;// stops ADC conversion
}

//***************************************************************************
//
// Function Name : "ADC_isConversionDone"
// Target MCU : AVR128DB48
// Target Hardware : None
// DESCRIPTION
// Reads the INTFLAGS register of ADC0 to determine if the
// ADC conversion is finished
//
// Inputs : None
//
// Outputs : uint8_t result: if result is all 0s, conversion is not finished,
// if result is 0x01, conversion has finished
//
//
//**************************************************************************
uint8_t ADC_isConversionDone(void)
{
	return (ADC0.INTFLAGS & ADC_RESRDY_bm);//checks if RESRDY bit is set
}

//***************************************************************************
//
// Function Name : "ADC_read"
// Target MCU : AVR128DB48
// DESCRIPTION
// Returns the digital value converted by the ADC
//
// Inputs : None
//
// Outputs : uint16_t result: the 16 bit conversion result from the
// result register of the ADC
//
//
//**************************************************************************
uint16_t ADC_read(void)
{
	return ADC0.RES;	// Reading result clears interrupt flag
}

//***************************************************************************
//
// Function Name : "ADC_channelSEL"
// Target MCU : AVR128DB48
// DESCRIPTION
// Selects 1 (single ended mode) or 2 channels (differential mode)
// for ADC0 to read the voltage from
//
// Inputs : uint8_t channel_sel: represents the channel to be
// read (for differential mode, this is the lower of the two channels
// to be read)
//
// Outputs : None
//
//
//**************************************************************************
void ADC_channelSEL(uint8_t channel_sel)
{
	/* If differential mode -> write both MUXPOS and MUXNEG registers, otherwise only write MUXPOS register */
	if (adc_mux_mode == 0x01)// if differential mode
	{
		ADC0.MUXPOS = channel_sel;
		if(channel_sel == 0 || channel_sel == 3)
		{
			ADC0.MUXNEG = channel_sel + 2;
		}
		else
		{
			ADC0.MUXNEG = channel_sel + 1;
		}
	}
	else// single-ended mode
	ADC0.MUXPOS = channel_sel;
}

//***************************************************************************
//
// Function Name : "batteryCell_read"
// Target MCU : AVR128DB48
// DESCRIPTION
// Starts a conversion on ADC0 for one battery in the quad pack,
// reads the result, and converts the result back to an
// analog voltage
//
// Inputs : uint8_t channel_sel: the battery in the quad pack to be
// read
//
// Outputs : float adc_value: the analog voltage of the battery
//
//
//**************************************************************************
float batteryCell_read(uint8_t channel_sel)
{
	ADC_channelSEL(channel_sel);
	ADC_startConversion();
	while(ADC_isConversionDone() != 0x01) ;
	ADC_stopConversion();
	adc_value = ADC_read();
	adc_value = adc_value >> 4;//the 12 bit result was left adjusted, so shift right 4 places to fix
	return (float)(((float)adc_value / 2048) * adc_vref * battery_voltage_divider_ratios); //finds Vpos - Vneg
}

