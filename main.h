/* Global variable and function declarations */
#ifndef MAIN_H_
#define MAIN_H_

#define B1_ADC_CHANNEL	0x00	// AIN0 -> PD0: Battery cell 1 positive terminal
#define B2_ADC_CHANNEL	0x05	// AIN5 -> PD5: Battery cell 2 positive terminal
#define B3_ADC_CHANNEL	0x02	// AIN2 -> PD2: Battery cell 3 positive terminal
#define B4_ADC_CHANNEL	0x03	// AIN3 -> PD3: Battery cell 4 positive terminal
#define GND_ADC_CHANNEL	0x40	// AIN -> GND
#define OPAMP_ADC_CHANNEL	0x0A	// AIN10 -> PE2: OPAMP 2 output

/* Display buffer for DOG LCD using sprintf(). 4 lines, 21 characters per line */
char dsp_buff[4][21];

/*Global variable Declarations*/
volatile uint8_t adc_mode;	// ADC conversion mode: 0x00 -> single-ended, 0x01 -> differential
volatile float adc_value;	// Analog Voltage read from ADC in volts
volatile float adc_vref;	// Reference voltage used by ADC in volts

volatile float UNLOADED_battery_voltages[4];	// UNLOADED Battery cell voltages from most recent test
volatile float LOADED_battery_voltages[4];		// LOADED Battery cell voltages from most recent test

/* This should be stored in nonvolatile memory, otherwise the information will be lost after powering OFF */
volatile float HISTORY_UNLOADED_battery_voltages[13][4];	// History of previous UNLOADED Battery cell voltages
volatile float HISTORY_LOADED_battery_voltages[13][4];		// History of previous LOADED Battery cell voltages

volatile uint8_t battery_voltage_divider_ratios;	// Voltage divider ratio used for measuring battery cells

volatile float load_current_amps;		// Load current value in Amps
volatile uint8_t current_sensing_voltage_divider_ratios;	// Voltage divider ratio used for measuring voltage across shunt
volatile float shunt_resistance_ohms;	// 0.8 milli-ohms
volatile uint8_t OPAMP_gain;	// Gain configuration for current sensing instrumentation amplifier

volatile uint8_t cursor;	// LCD cursor line position (1,2,3,4)
volatile uint8_t quad_pack_entry;	// quad pack entry that cursor is pointing to, row index for 13x4 history matrices

/* Program states for the local interface fsm*/
typedef enum {
	MAIN_MENU_STATE,
	TEST_STATE,
	VIEW_HISTORY_STATE
}  LOCAL_INTERFACE_FSM_STATES;

/* States for the fsm that performs the test procedure */
typedef enum {
	ERROR,					// Test attempted while no battery pack was connected
	TESTING,				// Perform LOADED and UNLOADED tests
	DISPLAY_RESULTS,		// Display test results on LCD
	DISCARD_RESULTS,		// Confirm that user would like to discard test results without saving
	SAVE_CURRENT_RESULTS,	// Confirm that user would like to save current test results
	SCROLL_SAVE_ENTRIES,	// Scroll through quad pack entries to save current test results
	OVERWRITE_RESULTS		// Confirm that user would like to overwrite previous test results
}  TEST_FSM_STATES;

/* States for the fsm that displays previous results */
typedef enum {
	SCROLL_PREVIOUS_RESULTS,	// Scroll through quad pack entries where previous test results are saved
	DISPLAY_PREVIOUS_RESULTS	// Display previous test results for quad pack that was selected
}  VIEW_HISTORY_FSM_STATES;

/* Push Button Input Types */
typedef enum {
	OK,		// User pressed OK pushbutton
	BACK,	// User pressed BACK pushbutton
	UP,		// User pressed UP pushbutton
	DOWN,	// User pressed DOWN pushbutton
	NONE	// Neutral PB state to prevent FSM functions from acting on wrong PB press
}  PB_INPUT_TYPE;

/* Current state variables for each fsm */
volatile LOCAL_INTERFACE_FSM_STATES LOCAL_INTERFACE_CURRENT_STATE;
volatile TEST_FSM_STATES TEST_CURRENT_STATE;
volatile VIEW_HISTORY_FSM_STATES VIEW_HISTORY_CURRENT_STATE;
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
void test_fsm(void);
void is_battery_connected(void);
void display_error_message (PB_INPUT_TYPE pb_type);
void perform_test(void);
void display_test_results(PB_INPUT_TYPE pb_type, float LOADED_results[4], float UNLOADED_results[4]);
void discard_test_results(PB_INPUT_TYPE pb_type);
void save_test_results(PB_INPUT_TYPE pb_type);
void scroll_previous_entries(PB_INPUT_TYPE pb_type);
void overwrite_previous_results(PB_INPUT_TYPE pb_type);
void display_quad_pack_entries(void);
void view_history_fsm(void);

#endif /* MAIN_H_ */