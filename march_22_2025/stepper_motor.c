#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <math.h>
#include "main.h"

#define STEP_PERIOD_US 10	// 10us STEP period => 100kHz STEP frequency

//***************************************************************************
//
// Function Name : "DRV8825_init"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function initializes the STEP and DIR IO pins
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void DRV8825_init(void)
{	
	/* STEP -> PC4, DIR -> PC5*/
	PORTC.DIR |= (PIN4_bm | PIN5_bm);	// Configure STEP & DIR pins as outputs
	PORTC.OUT &= ~(PIN4_bm | PIN5_bm);	// Initialize both logic levels to LOW
}
//***************************************************************************
//
// Function Name : "DRV8825_step"
// Target MCU : AVR128DB48
// DESCRIPTION
// Triggers a rising edge pulse to step the DRV8825
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void DRV8825_step(void)
{	
	PORTC.OUT |= PIN4_bm;			// Rising edge on STEP pin
	_delay_us(0.5*STEP_PERIOD_US);	// delay for half PERIOD
	PORTC.OUT &= ~PIN4_bm;			// Falling edge on STEP pin
	_delay_us(0.5*STEP_PERIOD_US);	// delay for half PERIOD
}
//***************************************************************************
//
// Function Name : "DRV8825_dir_HIGH"
// Target MCU : AVR128DB48
// DESCRIPTION
// Sets the DIR pin to HIGH for the stepper motor direction
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void DRV8825_dir_HIGH(void)
{	
	PORTC.OUT |= PIN5_bm;
}

//***************************************************************************
//
// Function Name : "DRV8825_dir_LOW"
// Target MCU : AVR128DB48
// DESCRIPTION
// Sets the DIR pin to LOW for the stepper motor direction
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void DRV8825_dir_LOW(void)
{	
	PORTC.OUT &= ~PIN5_bm;
}

//***************************************************************************
//
// Function Name : "set_load_current"
// Target MCU : AVR128DB48
// DESCRIPTION
// Continuously adjusts the stepper motor position until the load current 
//	drawn from the battery is equal to the programmed value in amps
//
// Inputs : float target_current_amps
//
// Outputs : none
//
//**************************************************************************
void set_load_current(float target_current_amps)
{	
	load_current_amps = load_current_Read();
	float error = load_current_amps - target_current_amps;	// error between measured current and target current

	/* Remain in while loop until load current = target current +/- 10 amps */
	while(fabs(error) > 10)
	{
		/* Poll the load current reading from the shunt */
		load_current_amps = load_current_Read();
		error = load_current_amps - target_current_amps;
		
		/* Turn knob CLOCK-WISE if load current is LESS than target value*/
		if (error <= 0) { DRV8825_dir_LOW(); }
		/* Turn knob COUNTER-CLOCK-WISE if load current is MORE than target value*/
		else if (error >= 0) { DRV8825_dir_HIGH(); }
		
		/* Rotate the knob by one step of the NEMA-17 on each iteration */
		DRV8825_step();
	}
}

//***************************************************************************
//
// Function Name : "open_circuit_load"
// Target MCU : AVR128DB48
// DESCRIPTION
// Sets the load to an open circuit so zero amps are drawn from the battery
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void open_circuit_load(void)
{	
	DRV8825_dir_HIGH();	// rotate knob COUNTER-CLOCK-WISE
	load_current_amps = load_current_Read();
	
	/* Rotate knob until current is at minimum measurable value */
	while(load_current_amps > 10)
	{
		/* Poll the load current reading from the shunt */
		load_current_amps = load_current_Read();
		/* Rotate the knob by one step of the NEMA-17 on each iteration */
		DRV8825_step();
	}
	
	/* Complete one more rotation to ensure carbon pile is completely OFF */
	for(uint8_t i = 0; i < 200; i++) { DRV8825_step();	}
}
