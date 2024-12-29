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
// Initializes the ADC0 module of the AVR128DB48 for differential or
// single-ended mode, VDD reference, 12-bit resolution, free-run mode,
// 16 sample accumulation, and clock prescalar divided by 4.
// Enables interrupts and ADC0 module.
//
// Inputs : 
//		uint8_t mode: 0 -> single ended, 1 -> differential
//
// Outputs : None
//
//**************************************************************************
void ADC_init(uint8_t mode)
{
	adc_mode = mode;	// single-ended or differential mode
	
	// Use VDD as reference
	VREF.ADC0REF = VREF_REFSEL_VDD_gc;
	
	// 12-bit resolution, Free-Run mode, differential/single-ended, Right adjusted, Enable
	ADC0.CTRLA = (ADC_RESSEL_12BIT_gc | ADC_FREERUN_bm | (adc_mode << 5) | ADC_ENABLE_bm);

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
	ADC0.COMMAND = ADC_SPCONV_bm;	// stops ADC conversion
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
	return (ADC0.INTFLAGS & ADC_RESRDY_bm);	//checks if RESRDY bit is set
}
//***************************************************************************
//
// Function Name : "ADC_channelSEL"
// Target MCU : AVR128DB48
// DESCRIPTION
// Selects 1 (single ended mode) or 2 channels (differential mode)
//	for ADC0 to read the voltage from
//
// Inputs : 
//		uint8_t AIN_POS: Positive differential or single-ended input
//		uint8_t AIN_NEG: Negative differential input
//
// Outputs : None
//
//
//**************************************************************************
void ADC_channelSEL(uint8_t AIN_POS, uint8_t AIN_NEG)
{
	/* In differential mode, write both MUXPOS and MUXNEG registers */
	if (adc_mode == 0x01)
	{
		ADC0.MUXPOS = AIN_POS;
		ADC0.MUXNEG = AIN_NEG;
	}
	/* In single-ended mode, only write MUXPOS register */
	else
		ADC0.MUXPOS = AIN_POS;
}
//***************************************************************************
//
// Function Name : "ADC_read"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Reads an integer value from the ADC and converts it to a floating point
//	voltage based on reference voltage and conversion resolution
//
// Inputs : None
//
// Outputs : 
//		float result: A floating point voltage converted by the ADC
//
//**************************************************************************
float ADC_read(void)
{
	/* Perform ADC conversion */
	ADC_startConversion();
	while(ADC_isConversionDone() != 0x01);	// wait for conversion to finish
	ADC_stopConversion();
	
	/* 12 bit result was left adjusted, so shift right 4 places to fix, reading ADC.RES clears interrupt flag */
	if (adc_mode == 0x00)		
		return (float)( adc_vref * (ADC0.RES >> 4) / 4096);	// single-ended resolution is 12 bits -> 4096 values
	else
		return (float)( adc_vref * (ADC0.RES >> 4) / 2048);	// differential resolution is 11 bits -> 2048 values
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
// Inputs : 
//	uint8_t BAT_POS: Positive battery terminal
//	uint8_t BAT_NEG: Negative battery terminal
//
// Outputs :
//	float result: the analog voltage across the battery
//
//**************************************************************************
float batteryCell_read(uint8_t BAT_POS, uint8_t BAT_NEG)
{
	/* Put ADC in differential mode and select POS and NEG inputs*/
	ADC_init(0x01);	
	ADC_channelSEL(BAT_POS, BAT_NEG);
	
	/* Multiply by voltage divider ratio to undo attenuation */
	return (float) (ADC_read() * battery_voltage_divider_ratios);
}
//***************************************************************************
//
// Function Name : "read_UNLOADED_battery_voltages"
// Target MCU : AVR128DB48
// DESCRIPTION
//  Reads the voltage across each battery cell input and stores the results 
//	in the UNLOADED_battery_voltgaes array
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void read_UNLOADED_battery_voltages(void)
{
	/* Read voltage of each cell and store in array when unloaded */
	UNLOADED_battery_voltages[0] = batteryCell_read(B1_ADC_CHANNEL, GND_ADC_CHANNEL);	// B1_POS - GND
	UNLOADED_battery_voltages[1] = batteryCell_read(B2_ADC_CHANNEL, B1_ADC_CHANNEL);	// B2_POS - B1_POS
	UNLOADED_battery_voltages[2] = batteryCell_read(B3_ADC_CHANNEL, B2_ADC_CHANNEL);	// B3_POS - B2_POS
	UNLOADED_battery_voltages[3] = batteryCell_read(B4_ADC_CHANNEL, B3_ADC_CHANNEL);	// B4_POS - B3_POS
}
//***************************************************************************
//
// Function Name : "read_LOADED_battery_voltages"
// Target MCU : AVR128DB48
// DESCRIPTION
//  Reads the voltage across each battery cell input and stores the results
//	in the LOADED_battery_voltgaes array
// Inputs : none
//
// Outputs : none
//
//
//**************************************************************************
void read_LOADED_battery_voltages(void)
{
	/* Read voltage of each cell and store in array once load current reaches 500A */
	LOADED_battery_voltages[0] = batteryCell_read(B1_ADC_CHANNEL, GND_ADC_CHANNEL);	// B1_POS - GND
	LOADED_battery_voltages[1] = batteryCell_read(B2_ADC_CHANNEL, B1_ADC_CHANNEL);	// B2_POS - B1_POS
	LOADED_battery_voltages[2] = batteryCell_read(B3_ADC_CHANNEL, B2_ADC_CHANNEL);	// B3_POS - B2_POS
	LOADED_battery_voltages[3] = batteryCell_read(B4_ADC_CHANNEL, B3_ADC_CHANNEL);	// B4_POS - B3_POS
}
