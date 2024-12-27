#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

void Fan_PWM_init(void)
{
	// Enable TCA0 and set PWM frequency = 25kHz
	TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_DIV1_gc | 0x01);	// Use system clk
	TCA0.SINGLE.PER = 0xA0;	// TOP = 160 clk period @ 4MHz clk => 25kHz pwm frequency
	
	// Enable single-slope PWM waveform generation on compare channel 0
	TCA0.SINGLE.CTRLB = (TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm);
	
	// Default duty cycle = 0%
	TCA0.SINGLE.CMP0BUF = 0x00;
	
	PORTMUX.TCAROUTEA= 0x2;
	// Configure PB0 pin for PWM output
	PORTC.DIR |= PIN0_bm;
}

void set_Fan_PWM(uint8_t duty)
{
	// Duty Cycle = [100 - 100*[TOP - CMP]/TOP] %
	// CMP value = duty*(TOP/100)
	TCA0.SINGLE.CMP0BUF = duty * 1.6;
}

