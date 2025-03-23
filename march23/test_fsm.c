#include "main.h"

//***************************************************************************
//
// Function Name : "test_fsm"
// Target MCU : AVR128DB48
// DESCRIPTION
// Test finite state machine. This finite state machine has multiple states 
//  and it is responsible for performing the loaded and unloaded battery 
//  tests. It is also responsible for saving or discarding test results. 
//  The function uses a switch statement to call different functions based 
//	on the current state of the test. Note the push-button and test current
//	state variables are global variables declared in the header file. 
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void test_fsm(void)
{	
	switch (TEST_CURRENT_STATE)
	{
		case ERROR:
			display_error_message(PB_PRESS);
			break;
		case TESTING:
			perform_test();
			break;
		case SCROLL_TEST_RESULT_MENU_T:
			scroll_test_result_menu(PB_PRESS, current_test_result);
			break;
		case VOLTAGE_READINGS_T:
			if (PB_PRESS == BACK)
				TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;
			else
				display_voltage_readings(current_test_result);
			break;
		case HEALTH_RATINGS_T:
			if (PB_PRESS == BACK)
				TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;
			else
				display_health_ratings(current_test_result);
			break;
		case TEST_CONDITIONS_T:
			if (PB_PRESS == BACK)
				TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;
			else
				display_test_conditions(current_test_result);
			break;
		case DISCARD_RESULTS_T:
			discard_test_results(PB_PRESS);
			break;	
		case SAVE_CURRENT_RESULTS:
			save_test_results(PB_PRESS);
			break;
		case SCROLL_SAVE_ENTRIES:
			scroll_previous_entries(PB_PRESS);
			break;
		case OVERWRITE_RESULTS:
			overwrite_previous_results(PB_PRESS);			
			break;
		default:
			break;
	}

	return;
}
//***************************************************************************
//
// Function Name : "scroll_test_result_menu"
// Target MCU : AVR128DB48
// DESCRIPTION
// Allows the user to scroll through a menu to select with data from the 
//	quad pack test to display on the screen. The user can select to view the
//	loaded and unloaded voltages, the health ratings, or the test conditions.
//	The user also has the option to discard a test in this screen.
//
// Inputs : PB_INPUT_TYPE pb_type   : Pushbutton press identifier 
//			test_result result_data : test result data struct
//
// Outputs : none
//
//**************************************************************************
void scroll_test_result_menu(PB_INPUT_TYPE pb_type, test_result result_data)
{
	switch (pb_type)
	{
		/* UP pushbutton press -> Move cursor up and display results menu */
		case UP: 
			move_cursor_up();
			display_result_menu();
			break;
		/* DOWN pushbutton press -> Move cursor down and display results menu */
		case DOWN: 
			move_cursor_down();	
			display_result_menu();
			break;
		/* OK pushbutton press -> Display selected page of results menu */
		case OK:
			display_result_data(result_data);			
			break;
		/* BACK pushbutton press -> Confirm saving of results or return to scrolling through quad pack entries */
		case BACK:
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{
				// Proceed to next state -> save test results
				TEST_CURRENT_STATE = SAVE_CURRENT_RESULTS;
				save_test_results(NONE);		
			}
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				// Return to previous state -> Go back to scrolling through quadpack entries
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
				scroll_previous_entries(NONE);
			}
			break;			
		/* Default action is to display the results menu */
		default:
			display_result_menu();
			break;
	}	
}

//***************************************************************************
//
// Function Name : "display_result_data"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays the results of a quad pack test. Depending on 
//	which attribute of the data was selected from the results menu.
//
// Inputs  : test_result result_data : test result data struct
//
// Outputs : none
//
//**************************************************************************
void display_result_data(test_result result_data)
{
	/* Update display based on cursor position */
	switch (cursor)
	{
		/* LCD line 1: Display voltage measurements */
		case 1:	
			display_voltage_readings(result_data);		
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = VOLTAGE_READINGS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = VOLTAGE_READINGS_H;
			
			break;			
		/* LCD line 2: Display health ratings*/
		case 2:	
			display_health_ratings(result_data);
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = HEALTH_RATINGS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = HEALTH_RATINGS_H;
			
			break;
		/* LCD line 3: Display test conditions */
		case 3:
			display_test_conditions(result_data);
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = TEST_CONDITIONS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = TEST_CONDITIONS_H;
				
			break;
		/* LCD line 4: Confirm permanent discarding of results */
		case 4:
			discard_test_results(NONE);
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = DISCARD_RESULTS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = DISCARD_RESULTS_H;
			
			break;
		/* Default action is to do nothing */
		default:
			asm volatile("nop");
			break;
	}	
}

//***************************************************************************
//
// Function Name : "display_test_conditions"
// Target MCU : AVR128DB48
// DESCRIPTION
// Display the conditions under which the test was performed. This includes
//	the max load current that was drawn, the ambient temperature during the
//	test, the date of the test, and whether it was a manual test or an 
//	automated test.
//
// Inputs  : test_result result_data : test result data struct
//
// Outputs : none
//
//**************************************************************************
void display_test_conditions(test_result result)
{
	clear_lcd();
	sprintf(dsp_buff[0], "Load Current: %uA", result.max_load_current);
	sprintf(dsp_buff[2], "Amb Temp: %uC", result.ampient_temp);
	sprintf(dsp_buff[3], "Date: 20%u/%u/%u", result.year, result.month, result.day);
	if (result.test_mode == 0x00)
		sprintf(dsp_buff[1], "Mode: Manual");
	else
		sprintf(dsp_buff[1], "Mode: Automated");
	update_lcd();
}
//***************************************************************************
//
// Function Name : "display_voltage_readings"
// Target MCU : AVR128DB48
// DESCRIPTION
// Display the loaded and unloaded battery cell voltages from the test. The
// unloaded voltages are on the left column and the unloaded voltages are 
//	on the right column.
//
// Inputs  : test_result result_data : test result data struct
//
// Outputs : none
//
//**************************************************************************
void display_voltage_readings(test_result result) 
{
	clear_lcd();
	sprintf(dsp_buff[0], "B1: %.3f  B1: %.3f", result.UNLOADED_battery_voltages[0], result.LOADED_battery_voltages[0]);
	sprintf(dsp_buff[1], "B2: %.3f  B2: %.3f", result.UNLOADED_battery_voltages[1], result.LOADED_battery_voltages[1]);
	sprintf(dsp_buff[2], "B3: %.3f  B3: %.3f", result.UNLOADED_battery_voltages[2], result.LOADED_battery_voltages[2]);
	sprintf(dsp_buff[3], "B4: %.3f  B4: %.3f", result.UNLOADED_battery_voltages[3], result.LOADED_battery_voltages[3]);
	update_lcd();
}

//***************************************************************************
//
// Function Name : "decode_health_rating"
// Target MCU : AVR128DB48
// DESCRIPTION
// Assigns health ratings to a quad pack based on the loaded voltages. This
//	function calculates the largest threshold from the grading table that the
//	loaded voltage is greater than or equal to and assigns a health rating. 
//	An index is then used to load the string for that health rating from a 
//	look-up table. 
//
// Inputs  : test_result result_data : test result data struct
//
// Outputs : none
//
//**************************************************************************
void decode_health_rating(test_result result)
{
	uint8_t lut_idx = 0;	// index to lut containing health rating strings
	float rating_threshold = 2.9;	// minimum threshold for A = 2.9
	
	/* Determine health rating of all 4 battery cells in the quad pack */
	for (uint8_t i = 0; i < 4; i++)		// outer for loop, 4 battery cells
	{
		/* Loop until threshold falls below 1.69, F, or loaded voltage no longer meets minimum threshold */
		while ( (rating_threshold > 1.69) && (rating_threshold >= result.LOADED_battery_voltages[i]) )
		{
			/* Compare loaded voltage with threshold to determine health rating */
			if (result.LOADED_battery_voltages[i] < rating_threshold)
			{
				lut_idx++;	// Incrementing lut index lowers rating
				rating_threshold -= 0.1;	// lower threshold to compare with a lower health rating
			}
		}	
		
		/* Copy health rating strings from look-up table into buffer array */
		for (uint8_t j = 0; i < 3; j++)	// inner for loop, 3 characters per health rating
		{
			health_rating_characters[(3*i) + j] = health_rating_lut[lut_idx][j];
		}		
	}
}

//***************************************************************************
//
// Function Name : "display_health_ratings"
// Target MCU : AVR128DB48
// DESCRIPTION
// Calls the function to assign health ratings to the battery cells and then
//	displays them on the screen.
//
// Inputs  : test_result result_data : test result data struct
//
// Outputs : none
//
//**************************************************************************
void display_health_ratings(test_result result)
{
	/* Write health ratings into character buffer */
	decode_health_rating(result);
	
	/* Update display, array index mapping of char buffer: [0:2]->B1, [3:5]->B2, [6:8]->B3, [9:11]->B4 */
	clear_lcd();
	sprintf(dsp_buff[0], "B1: %c%c%c", health_rating_characters[0],health_rating_characters[1], health_rating_characters[2]);
	sprintf(dsp_buff[1], "B2: %c%c%c", health_rating_characters[3],health_rating_characters[4], health_rating_characters[5]);
	sprintf(dsp_buff[2], "B3: %c%c%c", health_rating_characters[6],health_rating_characters[7], health_rating_characters[8]);
	sprintf(dsp_buff[3], "B4: %c%c%c", health_rating_characters[9],health_rating_characters[10], health_rating_characters[11]);
	update_lcd();
}

//***************************************************************************
//
// Function Name : "display_result_menu"
// Target MCU : AVR128DB48
// DESCRIPTION
// Displays the menu where the user selects which attribute of the quad
//	pack result data to display on the screen.
//
// Inputs  : none
//
// Outputs : none
//
//**************************************************************************
void display_result_menu(void)
{
	clear_lcd();
	
	/* Result menu strings */
	sprintf(dsp_buff[0], "Voltage Readings    ");
	sprintf(dsp_buff[1], "Health Ratings      ");
	sprintf(dsp_buff[2], "Test Conditions     ");
	sprintf(dsp_buff[3], "Discard results	  ");

	/* Append cursor icon to end of string */
	dsp_buff[cursor - 1][19] = '<';
	dsp_buff[cursor - 1][20] = '-';
	
	update_lcd();
}

//***************************************************************************
//
// Function Name : "save_test_results"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays a message asking if the user would like to save
//  the results of the most recently completed test. Also updates the
//  state of the program to scroll through previous test results or
//  to display the most recent test results
//
// Inputs  : PB_INPUT_TYPE pb_type : pushbutton type identifier
//
// Outputs : none
//
//**************************************************************************
void save_test_results(PB_INPUT_TYPE pb_type)
{
	/* OK PB press to show entries to save results */
	if (pb_type == OK)
	{
		cursor = 1;		// Initialize cursor to line 1
		quad_pack_entry = 0;	// Initialize quad pack entry to quad pack 1
		TEST_CURRENT_STATE = SCROLL_SAVE_ENTRIES;
		scroll_previous_entries(NONE);
	}
	/* BACK PB press to go back to displaying current results*/
	else if (pb_type == BACK)
	{
		TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;
		scroll_test_result_menu(NONE, current_test_result);
 	}
	 /* Display message otherwise */
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Save results?       ");
		sprintf(dsp_buff[1], "Press OK            ");
		sprintf(dsp_buff[2], "Otherwise press BACK");
		update_lcd();
	}
}

//***************************************************************************
//
// Function Name : "overwrite_previous_results"
// Target MCU : AVR128DB48
// DESCRIPTION
//	This function displays a message asking if the user would like to
//  overwrite the results of a previous test and replace it with the results
//  of the most recently completed test. Results are written into EEPROM.
//
// Inputs  : PB_INPUT_TYPE pb_type : pushbutton type identifier
//
// Outputs : none
//
//**************************************************************************
void overwrite_previous_results(PB_INPUT_TYPE pb_type)
{
	/* OK pushbutton press to overwrite results and return to main menu */
	if (pb_type == OK)
	{
		/* Store data in EEPROM slot pointed to be quad pack entry index */
		eeprom_update_block(&current_test_result, &test_results_history_eeprom[quad_pack_entry], sizeof(test_result));		
		/* Return to main menu */
		cursor = 1;		// Initialize cursor to line 1
		quad_pack_entry = 0;	// Initialize quad pack entry to 1. Row index to 2D array, 0 is index to 1st entry
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	/* BACK pushbutton press to go back to scrolling through quad pack entries */
	else if (PB_PRESS == BACK)
	{
		TEST_CURRENT_STATE = SCROLL_SAVE_ENTRIES;
		scroll_previous_entries(NONE);
	}
	/* Display message otherwise */
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Press OK to          ");
		sprintf(dsp_buff[1], "overwrite old results");
		sprintf(dsp_buff[2], "Press BACK to        ");
		sprintf(dsp_buff[3], "view current results ");
		update_lcd();
	}
}

//***************************************************************************
//
// Function Name : "is_battery_connected"
// Target MCU : AVR128DB48
// DESCRIPTION
// Determines whether or not a battery is connected to the load analyzer. 
//	If the voltage across the battery inputs is less than 100mV, then no
//	battery is connected. This function must be called before entering the 
//	TEST_FSM because it sets the initial state to ERROR or TESTING
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void is_battery_connected(void)
{
	/* Read total battery pack voltage with single-ended measurement */
	ADC_init(0x00);
	ADC_channelSEL(B4_ADC_CHANNEL, GND_ADC_CHANNEL);
	float voltage = ADC_read() * battery_voltage_divider_ratios;
			
	/* If voltage < 0.1V, no battery connection -> move to ERROR state */
	if (voltage < 0.1)
		TEST_CURRENT_STATE = ERROR;	// Move to ERROR state
	/* Otherwise proceed with test */
	else
		TEST_CURRENT_STATE = TESTING;	// Proceed with TEST

	return;
}
//***************************************************************************
//
// Function Name : "display_error_message"
// Target MCU : AVR128DB48
// DESCRIPTION
// Displays an error message indicating that there is no battery connected
//  to the load analyzer. This function exits the ERROR state once the OK
//  or BACK pushbuttons are pressed.
//
// Inputs :
//		PB_INPUT_TYPE pb_type: pushbutton input type
//
// Outputs : none
//
//**************************************************************************
void display_error_message (PB_INPUT_TYPE pb_type)
{
	/* Return to main menu if OK or BACK PB is pressed */
	if (pb_type == OK || pb_type == BACK)
	{		
		cursor = 1;		// Initialize cursor to line 1
		quad_pack_entry = 0;	// Initialize quad pack entry to quad pack 1
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	/* Display Error message otherwise */
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Failed! Ensure      ");
		sprintf(dsp_buff[1], "Proper Connection   ");
		sprintf(dsp_buff[2], "Press OK or BACK    ");
		sprintf(dsp_buff[3], "to Continue         ");
		update_lcd();
	}
}
//***************************************************************************
//
// Function Name : "perform_test"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function performs the loaded and unloaded tests. It reads the 
//  unloaded voltages and prompts the user to rotate the knob to draw 500A. 
//	It then reads the loaded battery voltages and beeps until the user turns
//	the knob back to the unloaded state. This function automatically changes
//	the current test state to display results regardless of the pushbutton 
//  press. 
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void perform_test(void)
{
	// read voltage of each cell and store in array when unloaded
	read_UNLOADED_battery_voltages();
	
	// Turn ON fan to prevent overheating
	set_Fan_PWM(75);
	
	// Read load current and wait until it hits 500A +/- 20A error
	load_current_amps = load_current_Read();
	
	/* Tell user to rotate knob of carbon pile until beep indicates 500A... */
	clear_lcd();
	sprintf(dsp_buff[0], "Rotate Knob until   "); 
	sprintf(dsp_buff[1], "beeping sound is    ");
	sprintf(dsp_buff[2], "heard...            ");
	sprintf(dsp_buff[3], "Load Current: %.1fA ", load_current_amps);
	update_lcd();
	
	while (load_current_amps < 500)	// infinite loop until current reaches 500A
	{		
		/* Update current reading on display if changes by more than 2% */ 
		load_current_amps = load_current_Read();
		
		_delay_ms(50);	// delay to prevent LCD to updating too fast
		clear_lcd();
		sprintf(dsp_buff[0], "Rotate Knob until   "); 
		sprintf(dsp_buff[1], "beeping sound is    ");
		sprintf(dsp_buff[2], "heard...            ");
		sprintf(dsp_buff[3], "Load Current: %.1fA ", load_current_amps);
		update_lcd();
	}
		
	// read voltage of each cell and store in array once load current reaches 500A
	read_LOADED_battery_voltages();
	_delay_ms(1000);

	/* Tell user to turn off carbon pile load... */
	clear_lcd();
	sprintf(dsp_buff[0], "Test complete...    ");
	sprintf(dsp_buff[1], "Rotate Knob until   ");
	sprintf(dsp_buff[2], "beeping stops...    ");	
	sprintf(dsp_buff[3], "Load Current: %.1fA ", load_current_amps);
	update_lcd();

	/* Make buzzer beep until current is below 50A */	
	while (load_current_amps > 200)
	{	
		/* Update current reading on display if changes by more than 2% */
		load_current_amps = load_current_Read();

		_delay_ms(50);
		clear_lcd();
		sprintf(dsp_buff[0], "Test complete...    ");
		sprintf(dsp_buff[1], "Rotate Knob until   ");
		sprintf(dsp_buff[2], "beeping stops...    ");
		sprintf(dsp_buff[3], "Load Current: %.1fA ", load_current_amps);
		update_lcd();		
		
		buzzer_ON();		
		_delay_ms(1000);	    // wait 1 second
		buzzer_OFF();
		_delay_ms(1000);		// wait 1 second
	}
		
	set_Fan_PWM(0);	// Turn fan OFF

	// Proceed to next state -> display test results
	TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;	
	display_result_menu();
}