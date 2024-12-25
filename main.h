/*
 * main.h
 *
 * Created: 12/18/2024 4:00:42 PM
 *  Author: tyler
 */ 


#ifndef MAIN_H_
#define MAIN_H_

// Display buffer for DOG LCD using sprintf()
char dsp_buff1[21];
char dsp_buff2[21];
char dsp_buff3[21];
char dsp_buff4[21];

/*Global variable Declarations*/
volatile uint16_t adc_value;		// Integer value read from ADC
volatile float UNLOADED_battery_voltages[4];	// UNLOADED Battery cell voltages
volatile float LOADED_battery_voltages[4];		// LOADED Battery cell voltages
volatile float HISTORY_UNLOADED_battery_voltages[4][13];	// UNLOADED Battery cell voltages
volatile float HISTORY_LOADED_battery_voltages[4][13];		// LOADED Battery cell voltages

volatile float adc_vref;		// Reference voltage of 3.3 V
volatile float load_current;		// Load current value in Amps
volatile int battery_voltage_divider_ratios;	// Voltage divider ratio used for measuring battery cells
volatile int current_sensing_voltage_divider_ratios;	// Voltage divider ratio used for measuring voltage across shunt
volatile uint16_t adc_mux_mode;	// 0 -> Single-ended, 1 -> Differential
volatile float shunt_resistance_ohms;
volatile float OPAMP_gain = 15;	// Gain of current sensing instrumentation amplifier
volatile uint8_t cursor_line_position = 1;	// is cursor on line 1,2,3, or 4 of LCD, 0 => cursor OFF
volatile uint8_t screen_state;			// screen state, i.e., homescreen, loading screen, etc.

// LCD Functions
void lcd_spi_transmit (char cmd); //transmits character using spi
void init_spi_lcd (void); //initializes spi module of AVR128DB48
void init_lcd (void); //initializes lcd
void update_lcd(void); //updates lcd
void clear_lcd (void);

// ADC Functions
void ADC_init(void);	// Initializes ADC
void ADC_startConversion(void);	// Starts a conversion by the ADC
void ADC_stopConversion(void);	// Stops a conversion by the ADC
void ADC_channelSEL(uint8_t channel_sel);	// Selects ADC channel when in differential mode
uint8_t ADC_isConversionDone(void);	// Checks if ADC conversion is finished
uint16_t ADC_read(void);	// Returns result from ADC
float batteryCell_read(uint8_t channel_sel); // reads one battery cell

// Battery voltage Functions
void display_battery_voltages_UNLOADED(void);
void display_battery_voltages_LOADED(void);
void conduct_test(void);

// OPAMP and current sensing Functions
void OPAMP_Instrumentation_init(void);
float get_OPAMP_gain(void);
float load_current_Read(void);

// Fan Functions
void Fan_PWM_init(void);
void set_Fan_PWM(uint8_t duty);

// Local Interface Functions
void UP_ISR(void);
void DOWN_ISR(void);
void BACK_ISR(void);
void OK_ISR(void);

void display_MAIN_MENU(void);
void display_MESSAGE1(void);	/* Prompt user to rotate knob clockwise until beeping indicates 500A */
void display_MESSAGE2(void);	/* Prompt user to rotate knob counter-clockwise until beeping stops */

#endif /* MAIN_H_ */