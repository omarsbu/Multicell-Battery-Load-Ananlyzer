#include "main.h"

volatile char health_rating_lut[13][3] = {
	"A  ", "A- ",					// 0x00, 0x01
	"B+ ", "B  ", "B- ",			// 0x02, 0x03, 0x04
	"C+ ", "C  ", "C- ", "C--",		// 0x05, 0x06, 0x07, 0x08
	"D+ ", "D  ", "D- ",			// 0x09, 0x0A, 0x0B
	"F  "							// 0x0C
};

test_result EEMEM test_results_history_eeprom[13];	// 507/512 bytes of available EEPROM

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
	
	/* Initialize Fan PWM module */
	Fan_PWM_init();
	
	/* Initialize pushbutton IO pins */
	PB_init();
	
	LOCAL_INTERFACE_FSM();

	sei(); // enable interrupts

	while(1)
	{
		asm volatile("nop");
	}
}
