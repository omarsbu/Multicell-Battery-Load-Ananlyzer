/* Global variable and function declarations */
#ifndef MAIN_H_
#define MAIN_H_

#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include<math.h>
#include <string.h>

#define B1_ADC_CHANNEL	0x00	// AIN0 -> PD0: Battery cell 1 positive terminal
#define B2_ADC_CHANNEL	0x06	// AIN5 -> PD5: Battery cell 2 positive terminal
#define B3_ADC_CHANNEL	0x07	// AIN2 -> PD2: Battery cell 3 positive terminal
#define B4_ADC_CHANNEL	0x03	// AIN3 -> PD3: Battery cell 4 positive terminal
#define GND_ADC_CHANNEL	0x40	// AIN -> GND
#define OPAMP_ADC_CHANNEL	0x0A	// AIN10 -> PE2: OPAMP 2 output

/* Display buffer for DOG LCD using sprintf(). 4 lines, 21 characters per line */
char dsp_buff[4][21];

/*Global variable Declarations*/
volatile uint8_t adc_mode;	// ADC conversion mode: 0x00 -> single-ended, 0x01 -> differential
volatile float adc_value;	// Analog Voltage read from ADC in volts
volatile float adc_vref;	// Reference voltage used by ADC in volts

volatile uint8_t battery_voltage_divider_ratios;	// Voltage divider ratio used for measuring battery cells

volatile float load_current_amps;		// Load current value in Amps
volatile uint8_t current_sensing_voltage_divider_ratios;	// Voltage divider ratio used for measuring voltage across shunt
volatile float shunt_resistance_ohms;	// 0.8 milli-ohms
volatile uint8_t OPAMP_gain;	// Gain configuration for current sensing instrumentation amplifier
volatile float temp;	// temporary variable

volatile uint8_t cursor;	// LCD cursor line position (1,2,3,4)
volatile uint8_t quad_pack_entry;	// quad pack entry that cursor is pointing to, row index for 13x4 history matrices

/* buffer array storing the health ratings of all 4 cells as strings, [0:2]->B1, [3:5]->B2, [6:8]->B3, [9:11]->B4 */
volatile char health_rating_characters[12];

/* Look-up table used to map the loaded voltages to a health rating string */
extern volatile char health_rating_lut[13][3];

// Settings global variables
volatile uint8_t testing_mode;
volatile uint16_t current_setting;
volatile uint8_t current_setting_100_dig;
volatile uint8_t current_setting_10_dig;
volatile uint8_t current_setting_1_dig;
volatile uint8_t voltage_precision;

typedef struct {
	float UNLOADED_battery_voltages[4];		// UNLOADED Battery cell voltages : 4 floats = 16 bytes
	float LOADED_battery_voltages[4];		// LOADED Battery cell voltages : 4 floats = 16 bytes
	uint16_t max_load_current;	// Max load current used to test battery : 2 bytes
	uint8_t test_mode;			// 0x00 -> Manual test, 0x01 -> Automated test : 1 byte
	uint8_t ampient_temp;		// Ambient temperature during test in degrees celcius : 1 bytes
	uint8_t year, month, day;	// 20xx, 0-12, 0-31 : 3 bytes
	// SIZE = 16 + 16 + 2 + 1 + 1 + 3 = 39 bytes
} test_result;

/* Data log of 13 previous quad-pack tests, stored in MCU's internal EEPROM storage */
extern test_result EEMEM test_results_history_eeprom[13];	// 507/512 bytes of available EEPROM
volatile test_result current_test_result;	// data from most recent quad-pack test



/* Program states for the local interface fsm*/
typedef enum {
	MAIN_MENU_STATE,
	TEST_STATE,
	VIEW_HISTORY_STATE,
	SETTINGS_STATE
}  LOCAL_INTERFACE_FSM_STATES;

/* States for the fsm that performs the test procedure */
typedef enum {
	ERROR,						// Test attempted while no battery pack was connected
	TESTING,					// Perform LOADED and UNLOADED tests
	SCROLL_TEST_RESULT_MENU_T,	// Menu to scroll through the data recorded during the test
	VOLTAGE_READINGS_T,			// Display LOADED (left) voltages and UNLOADED(right) voltages
	HEALTH_RATINGS_T,			// Display health ratings of battery cells
	TEST_CONDITIONS_T,			// Display conditions that the quad-pack was tested under
	DISCARD_RESULTS_T,			// Confirm that user would like to discard test results without saving
	SAVE_CURRENT_RESULTS,		// Confirm that user would like to save current test results
	SCROLL_SAVE_ENTRIES,		// Scroll through quad pack entries to save current test results
	OVERWRITE_RESULTS			// Confirm that user would like to overwrite previous test results
}  TEST_FSM_STATES;

/* States for the fsm that views previous results */
typedef enum {
	SCROLL_PREVIOUS_RESULTS,	// Scroll through quad pack entries where previous test results are saved
	SCROLL_TEST_RESULT_MENU_H,	// Menu to scroll through the data recorded during the test
	VOLTAGE_READINGS_H,			// Display LOADED (left) voltages and UNLOADED(right) voltages
	HEALTH_RATINGS_H,			// Display health ratings of battery cells
	TEST_CONDITIONS_H,			// Display conditions that the quad-pack was tested under
	DISCARD_RESULTS_H,			// Confirm that user would like to discard test results without saving
}  VIEW_HISTORY_FSM_STATES;

/* States for the fsm that displays the settings menu */
typedef enum {
	SCROLL_SETTINGS,					// Scroll through the settings menu
	LOAD_CURRENT_SETTINGS_SCREEN,		// Adjust the load current value used for automated tests
	VOLTAGE_PRECISION_SETTINGS_SCREEN	// Adjust the number of decimal points for voltage measurements
}  SETTINGS_FSM_STATES;

/* Push Button Input Types */
typedef enum {
	OK,		// User pressed OK pushbutton
	BACK,	// User pressed BACK pushbutton
	UP,		// User pressed UP pushbutton
	DOWN,	// User pressed DOWN pushbutton
	NONE	// Neutral PB state to prevent FSM functions from acting on wrong PB press
}  PB_INPUT_TYPE;

/* 24-bit unsigned integer type */
typedef struct {
	uint8_t upper;	// Upper byte:  [23:16]
	uint8_t middle;	// Middle byte: [15:8]
	uint8_t lower;	// Low byte:    [7:0]
} uint24_t;

/* Current state variables for each fsm */
volatile LOCAL_INTERFACE_FSM_STATES LOCAL_INTERFACE_CURRENT_STATE;
volatile TEST_FSM_STATES TEST_CURRENT_STATE;
volatile VIEW_HISTORY_FSM_STATES VIEW_HISTORY_CURRENT_STATE;
volatile SETTINGS_FSM_STATES SETTING_CURRENT_STATE;
volatile PB_INPUT_TYPE PB_PRESS;	// Always reset to NONE after handling a PB interrupt, eliminates ambiguity on next PB press

/* LCD Functions -> File Location: "lcd.c" */
void lcd_spi_transmit (char cmd); // transmits character using spi
void init_spi_lcd (void);	// initializes spi module of AVR128DB48
void init_lcd (void);	// initializes lcd
void update_lcd(void);	// updates lcd
void clear_lcd (void);	// clears lcd

/* ADC Functions -> File Location: "adc.c" */
void ADC_init(uint8_t mode);	// Initializes ADC, differential or single-ended
void ADC_startConversion(void);	// Starts a conversion by the ADC
void ADC_stopConversion(void);	// Stops a conversion by the ADC
uint8_t ADC_isConversionDone(void);	// Checks if ADC conversion is finished
void ADC_channelSEL(uint8_t AIN_POS, uint8_t AIN_NEG);	// Selects ADC channel
float ADC_read(void);	// Returns result from ADC
float batteryCell_read(uint8_t BAT_POS, uint8_t BAT_NEG); // reads voltage across 2 battery terminals
void read_UNLOADED_battery_voltages(void);	// reads 4 battery cells and stores in UNLOADED voltages array
void read_LOADED_battery_voltages(void);	// reads 4 battery cells and stores in LOADED voltages array

/* OPAMP and current sensing Functions -> File Location: "opamp.c" */
void OPAMP_Instrumentation_init(void);
float get_OPAMP_gain(void);
float load_current_Read(void);

/* Fan Functions -> File Location: "fan.c" */
void Fan_PWM_init(void);
void set_Fan_PWM(uint8_t duty);

/* Local Interface Functions -> File Location: "local_interface.c" */
void PB_init(void);
void buzzer_ON(void);
void buzzer_OFF(void);
void LOCAL_INTERFACE_FSM(void);
void UP_ISR(void);
void DOWN_ISR(void);
void BACK_ISR(void);
void OK_ISR(void);
void move_cursor_up(void);
void move_cursor_down(void);
void display_main_menu(void);
void main_menu_fsm(void);


void display_test_results(PB_INPUT_TYPE pb_type, test_result result);
void discard_test_results(PB_INPUT_TYPE pb_type);

/* View History FSM Functions -> File Location: "view_history.c" */
void view_history_fsm(void);
void scroll_previous_entries(PB_INPUT_TYPE pb_type);
void display_quad_pack_entries(void);

/* Settings FSM Functions -> File Location: "settings_fsm.c" */
void settings_fsm(void);
void scroll_settings_menu(PB_INPUT_TYPE pb_type);
void display_settings_menu(void);
void settings_menu_OK(void);
void display_load_current_setting(void);
void adjust_load_current_settings(PB_INPUT_TYPE pb_type);
void adjust_voltage_precision_settings(PB_INPUT_TYPE pb_type);

/* Test FSM Functions -> File Location: "test_fsm.c" */
void test_fsm(void);
void scroll_test_result_menu(PB_INPUT_TYPE pb_type, test_result result_data);
void save_test_results(PB_INPUT_TYPE pb_type);
void overwrite_previous_results(PB_INPUT_TYPE pb_type);
void is_battery_connected(void);
void display_error_message (PB_INPUT_TYPE pb_type);
void perform_test(void);



//-------------------------------/* move to local_interface.c file */-------------------------------------
void display_result_data(test_result result_data);
void display_test_conditions(test_result result);
void display_voltage_readings(test_result result);
void decode_health_rating(test_result result);
void display_health_ratings(test_result result);
void display_result_menu(void);





#endif /* MAIN_H_ */