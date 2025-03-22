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
//	on the current state of the test
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
		case SCROLL_QUADPACK_DATA_T:
			scroll_test_result_menu(PB_PRESS, current_test_result);
			break;
		case VOLTAGE_READINGS_T:
			if (PB_PRESS == BACK)
				TEST_CURRENT_STATE = SCROLL_QUADPACK_DATA_T;
			else
				display_voltage_readings(current_test_result);
			break;
		case HEALTH_RATINGS_T:
			if (PB_PRESS == BACK)
				TEST_CURRENT_STATE = SCROLL_QUADPACK_DATA_T;
			else
				display_health_ratings(current_test_result);
			break;
		case TEST_CONDITIONS_T:
			if (PB_PRESS == BACK)
				TEST_CURRENT_STATE = SCROLL_QUADPACK_DATA_T;
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
			overwrite_previous_results(PB_PRESS);			/* Rewrite to store data in EEPROM */
			break;
		default:
			break;
	}

	return;
}

void scroll_test_result_menu(PB_INPUT_TYPE pb_type, struct test_result result_data)
{
	switch (pb_type)
	{
		case UP: 
		/* Scroll up through quad pack results menu */
			move_cursor_up();
			display_result_menu();
			break;
		case DOWN: 
		/* Scroll down through quad pack results menu */
			move_cursor_down();	
			display_result_menu();
			break;
		case OK:
		/* Display selected page of results menu */
			display_result_data(result_data);			
			break;
		case BACK:
		/* Confirm saving of results or return to scrolling through quad pack entries */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{
				// Proceed to next state -> save test results
				TEST_CURRENT_STATE = SAVE_CURRENT_RESULTS;
				save_test_results(NONE);		
			}
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
				scroll_previous_entries(NONE);
			}
			break;			
		default:
			display_result_menu();
			break;
	}	
}

void display_result_data(struct test_result result_data)
{
	/* Update display based on cursor position */
	switch (cursor)
	{
		//-------------------------------------------------------------------//
		case 1:	
			/* LCD line 1: Display voltage measurements */
			display_voltage_readings(result_data);		
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = VOLTAGE_READINGS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = VOLTAGE_READINGS_H;
			break;			
		//-------------------------------------------------------------------//
		case 2:	
			/* LCD line 2: Display health ratings*/
			display_health_ratings(result_data);
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = HEALTH_RATINGS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = HEALTH_RATINGS_H;
			break;
		//-------------------------------------------------------------------//
		case 3:
			/* LCD line 3: Display test conditions */
			display_test_conditions(result_data);
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = TEST_CONDITIONS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = TEST_CONDITIONS_H;
			break;
		//-------------------------------------------------------------------//
		case 4:
			/* LCD line 4: Confirm permanent discarding of results */	
			discard_test_results(NONE);
			
			/* Update state variable of fsm */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
				TEST_CURRENT_STATE = DISCARD_RESULTS_T;
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
				VIEW_HISTORY_CURRENT_STATE = DISCARD_RESULTS_H;
			break;
		default:
			break;
	}	
}

void discard_test_results(PB_INPUT_TYPE pb_type)
{
	/* OK PB press to discard results & return to main menu */
	if (pb_type == OK)
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			current_test_result.LOADED_battery_voltages[i] = 0;
			current_test_result.UNLOADED_battery_voltages[i] = 0;
		}
		current_test_result.max_load_current = 0;
		current_test_result.ampient_temp = 0;
		current_test_result.test_mode = 0x00;
		current_test_result.year = 0;
		current_test_result.month = 0;
		current_test_result.day = 0;		
		
		/* Discard old test data */
		if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_CURRENT_STATE) {
			if (VIEW_HISTORY_CURRENT_STATE == DISCARD_RESULTS_H) {
				eeprom_update_block(&current_test_result, &test_results_history_eeprom[quad_pack_entry], sizeof(test_result));
			}
		}
		
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	/* BACK PB press to go back to displaying current results*/
	else if (pb_type == BACK)
	{
		if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
		{
			TEST_CURRENT_STATE = SCROLL_QUADPACK_DATA_T;
			scroll_test_result_menu(NONE, current_test_result);
		}
		else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
		{	
			VIEW_HISTORY_CURRENT_STATE = SCROLL_QUADPACK_DATA_H;
			scroll_test_result_menu(NONE, current_test_result);
		}
	}
	/* Otherwise display message */
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Press OK to         ");
		sprintf(dsp_buff[1], "permanently discard ");
		sprintf(dsp_buff[2], "test results, press ");
		sprintf(dsp_buff[3], "BACK to view results");
		update_lcd();
	}
}

void display_test_conditions(struct test_result result)
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

void display_voltage_readings(struct test_result result) 
{
	clear_lcd();
	sprintf(dsp_buff[0], "B1: %.3f  B1: %.3f", result.UNLOADED_battery_voltages[0], result.LOADED_battery_voltages[0]);
	sprintf(dsp_buff[1], "B2: %.3f  B2: %.3f", result.UNLOADED_battery_voltages[1], result.LOADED_battery_voltages[1]);
	sprintf(dsp_buff[2], "B3: %.3f  B3: %.3f", result.UNLOADED_battery_voltages[2], result.LOADED_battery_voltages[2]);
	sprintf(dsp_buff[3], "B4: %.3f  B4: %.3f", result.UNLOADED_battery_voltages[3], result.LOADED_battery_voltages[3]);
	update_lcd();
}

volatile char health_rating_characters[12]; // [0:2]->B1, [3:5]->B2, [6:8]->B3, [9:11]->B4 
volatile char health_rating_lut[13][3] = {
	"A  ", "A- ",					// 0x00, 0x01 
	"B+ ", "B  ", "B- ",			// 0x02, 0x03, 0x04
	"C+ ", "C  ", "C-  ", "C--",	// 0x05, 0x06, 0x07, 0x08
	"D+ ", "D  ", "D- ",			// 0x09, 0x0A, 0x0B
	"F  "							// 0x0C
};

void decode_health_rating(struct test_result result)
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

void display_health_ratings(struct test_result result)
{
	/* Map 16-bit code from EEPROM onto an array of 4 strings containing the health ratings */
	decode_health_rating(result);
	
	/* Update display, array index mapping: [0:2]->B1, [3:5]->B2, [6:8]->B3, [9:11]->B4 */
	clear_lcd();
	sprintf(dsp_buff[0], "B1: %c%c%c", health_rating_characters[0],health_rating_characters[1], health_rating_characters[2]);
	sprintf(dsp_buff[1], "B2: %c%c%c", health_rating_characters[3],health_rating_characters[4], health_rating_characters[5]);
	sprintf(dsp_buff[2], "B3: %c%c%c", health_rating_characters[6],health_rating_characters[7], health_rating_characters[8]);
	sprintf(dsp_buff[3], "B4: %c%c%c", health_rating_characters[9],health_rating_characters[10], health_rating_characters[11]);
	update_lcd();
}

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
		TEST_CURRENT_STATE = SCROLL_QUADPACK_DATA_T;
		scroll_test_result_menu(NONE, current_test_result);
 	}
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Save results?       ");
		sprintf(dsp_buff[1], "Press OK            ");
		sprintf(dsp_buff[2], "Otherwise press BACK");
		update_lcd();
	}
}

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