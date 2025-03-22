#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"
#include<math.h>
#include <string.h>

ISR(PORTA_PORT_vect)
{
	cli();

	if(VPORTA_INTFLAGS & PIN2_bm)
	{
		VPORTA_INTFLAGS |= PIN2_bm;
		OK_ISR();
	}
	else  if(VPORTA_INTFLAGS & PIN3_bm)
	{
		VPORTA_INTFLAGS |= PIN3_bm;
		BACK_ISR();
	}
	else if(VPORTA_INTFLAGS & PIN4_bm)
	{
		VPORTA_INTFLAGS |= PIN4_bm;
		UP_ISR();
	}
	else if(VPORTA_INTFLAGS & PIN5_bm)
	{
		VPORTA_INTFLAGS |= PIN5_bm;
		DOWN_ISR();
	}
	
	_delay_ms(300);
	
	sei();
}

//***************************************************************************
//
// Function Name : "OK_ISR"
// Target MCU : AVR128DB48
// DESCRIPTION
// Interrupt Service Routine for an OK pushbutton press
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void OK_ISR (void)
{
	/* Set pushbutton type to OK and enter local interface fsm */
	PB_PRESS = OK;
	LOCAL_INTERFACE_FSM();
	return;
}
//***************************************************************************
//
// Function Name : "BACK_ISR"
// Target MCU : AVR128DB48
// DESCRIPTION
// Interrupt Service Routine for an BACK pushbutton press
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void BACK_ISR (void)
{
	/* Set pushbutton type to BACK and enter local interface fsm */
	PB_PRESS = BACK;
	LOCAL_INTERFACE_FSM();
	return;
}
//***************************************************************************
//
// Function Name : "UP_ISR"
// Target MCU : AVR128DB48
// DESCRIPTION
// Interrupt Service Routine for an UP pushbutton press
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void UP_ISR (void)
{
	/* Set pushbutton type to UP and enter local interface fsm */
	PB_PRESS = UP;
	LOCAL_INTERFACE_FSM();
	return;
}
//***************************************************************************
//
// Function Name : "DOWN_ISR"
// Target MCU : AVR128DB48
// DESCRIPTION
// Interrupt Service Routine for an DOWN pushbutton press
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void DOWN_ISR (void)
{
	/* Set pushbutton type to DOWN and enter local interface fsm */
	PB_PRESS = DOWN;
	LOCAL_INTERFACE_FSM();
	return;
}
//***************************************************************************
//
// Function Name : "LOCAL_INTERFACE_FSM"
// Target MCU : AVR128DB48
// DESCRIPTION
// Top level Finite State Machine controller. This is the first function that 
//  is called after a pushbutton is pressed. Based on the current state of 
//  the local interface it will call either the main menu fsm, the test 
//  fsm, or the view history fsm. 
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void LOCAL_INTERFACE_FSM(void)
{
	switch (LOCAL_INTERFACE_CURRENT_STATE)
	{
		case MAIN_MENU_STATE:
			main_menu_fsm(); // Enter main menu fsm
			break;
		case TEST_STATE:		
			test_fsm();	// Enter test fsm
			break;
		case VIEW_HISTORY_STATE:
			view_history_fsm();	// Enter view history fsm
			break;
		case SETTINGS_STATE:
			settings_fsm();
		default:
			// return to main menu state
			LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
			main_menu_fsm();
			break;
	}
	
	PB_PRESS = NONE;	// Clear pushbutton state after it is handled
	_delay_ms(500);	// debounce
	return;
}
//***************************************************************************
//
// Function Name : "main_menu_fsm"
// Target MCU : AVR128DB48
// DESCRIPTION
// Main menu finite state machine. This finite state machine only has one 
//	state and based on which pushbutton was pressed, it either updates the 
//	cursor and its position on the display or it transfers control to the
//  test fsm or the view history fsm 
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void main_menu_fsm(void)
{
	/* Handle pushbutton press*/
	switch (PB_PRESS)
	{
		case OK:
			/* OK PB pressed while cursor was on 'Test' option */
			if (cursor == 1) 
			{
				/* Update program state variables and initiate test */
				LOCAL_INTERFACE_CURRENT_STATE = TEST_STATE;
				
				/* check whether to display error message or proceed with test fsm */
				is_battery_connected();	// initializes TEST_CURRENT_STATE to either ERROR or TESTING
				PB_PRESS = NONE;	// Clear pushbutton state before transitioning to new fsm
				test_fsm();	// Enter test fsm
			}
			/* OK PB pressed while cursor was on 'View History' option */
			else if (cursor == 2) 
			{
				/* Update program state variables */
				LOCAL_INTERFACE_CURRENT_STATE = VIEW_HISTORY_STATE;
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;	// initialize fsm state
				PB_PRESS = NONE;	// Clear pushbutton state before transitioning to new fsm
				view_history_fsm();	// Enter view history fsm
			}
			/* OK PB pressed while cursor was on 'Settings' option */
			else if (cursor == 3)
			{
				/* Update program state variables */
				LOCAL_INTERFACE_CURRENT_STATE = SETTINGS_STATE;	
				SETTINGS_STATE = SCROLL_SETTINGS;	
				PB_PRESS = NONE;	// Clear pushbutton state before transitioning to new fsm
				settings_fsm();	// Enter view settings fsm			
			}
			break;
		case BACK:
			asm volatile ("nop");	// do nothing
			break;
		case UP:
			move_cursor_up();
			display_main_menu();
			break;
		case DOWN:
			move_cursor_down();
			display_main_menu();
			break;
		default:
			display_main_menu();
			break;
	}

	return;
}
//***************************************************************************
//
// Function Name : "view_history_fsm"
// Target MCU : AVR128DB48
// DESCRIPTION
// View History finite state machine. This finite state machine is 
// responsible for scrolling through and displaying previous test results.
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void view_history_fsm(void)
{
	switch (VIEW_HISTORY_CURRENT_STATE)
	{
		case SCROLL_PREVIOUS_RESULTS:
			scroll_previous_entries(PB_PRESS);
			break;
		case DISPLAY_PREVIOUS_RESULTS:
			display_test_results(PB_PRESS, HISTORY_LOADED_battery_voltages[quad_pack_entry], HISTORY_UNLOADED_battery_voltages[quad_pack_entry]);
			break;
		default:
			break;
	}

	return;
}

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
		case DISPLAY_RESULTS:
			display_test_results(PB_PRESS, LOADED_battery_voltages, UNLOADED_battery_voltages);
			break;
		case DISCARD_RESULTS:
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
		temp = load_current_amps;
		load_current_amps = load_current_Read();
		temp = (load_current_amps - temp) / temp;
		
//		if (temp > 0.002|| temp < -0.002)
//		{
			_delay_ms(50);	// delay to prevent LCD to updating too fast
			clear_lcd();
			sprintf(dsp_buff[0], "Rotate Knob until   "); 
			sprintf(dsp_buff[1], "beeping sound is    ");
			sprintf(dsp_buff[2], "heard...            ");
			sprintf(dsp_buff[3], "Load Current: %.1fA ", load_current_amps);
			update_lcd();
//		}
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
		temp = load_current_amps;
		load_current_amps = load_current_Read();
		temp = (load_current_amps - temp) / temp;

//		if (temp > 0.002 || temp < -0.002)
//		{
			temp = load_current_amps;
			_delay_ms(50);
			clear_lcd();
			sprintf(dsp_buff[0], "Test complete...    ");
			sprintf(dsp_buff[1], "Rotate Knob until   ");
			sprintf(dsp_buff[2], "beeping stops...    ");
			sprintf(dsp_buff[3], "Load Current: %.1fA ", load_current_amps);
			update_lcd();
//		}		
		
		buzzer_ON();		
		_delay_ms(1000);	    // wait 1 second
		buzzer_OFF();
		_delay_ms(1000);		// wait 1 second
	}
		
	set_Fan_PWM(0);	// Turn fan OFF

	// Proceed to next state -> display test results
	TEST_CURRENT_STATE = DISPLAY_RESULTS;	
	display_test_results(NONE, LOADED_battery_voltages, UNLOADED_battery_voltages);
}
//***************************************************************************
//
// Function Name : "display_test_results"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays the results of the most recently completed test.
//  It also updates the test state to save or discard the results of the 
//  most recently completed test.
//
// Inputs : 
//		uint8_t LOADED_results[4]: loaded results to be displayed
//		uint8_t UNLOADED_results[4]: unloaded results to be displayed
//		PB_INPUT_TYPE pb_type: pushbutton input type
//
// Outputs : none
//
//**************************************************************************
void display_test_results(PB_INPUT_TYPE pb_type, float LOADED_results[4], float UNLOADED_results[4])
{
	/* OK PB press to save results */
	if (pb_type == OK)
	{
		if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
		{
			// Proceed to next state -> save test results
			TEST_CURRENT_STATE = SAVE_CURRENT_RESULTS;
			save_test_results(NONE);			
		}
		else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
		{
			return;
		}
	}
	/* BACK PB press to discard results*/
	else if (pb_type == BACK)
	{
		if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
		{
			// Proceed to next state -> discard test results
			TEST_CURRENT_STATE = DISCARD_RESULTS;
			discard_test_results(NONE);
		}
		else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
		{
			VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
			scroll_previous_entries(NONE);
		}	
	}
	/* Display results otherwise */
	else 
	{
		clear_lcd();
		sprintf(dsp_buff[0], "B1: %.3f  B1: %.3f", UNLOADED_results[0] ,LOADED_results[0]);
		sprintf(dsp_buff[1], "B2: %.3f  B2: %.3f", UNLOADED_results[1] ,LOADED_results[1]);
		sprintf(dsp_buff[2], "B3: %.3f  B3: %.3f", UNLOADED_results[2] ,LOADED_results[2]);
		sprintf(dsp_buff[3], "B4: %.3f  B4: %.3f", UNLOADED_results[3] ,LOADED_results[3]);
		update_lcd();
	}
}
//***************************************************************************
//
// Function Name : "discard_test_results"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays a message asking if the user would like to delete
//  the results of the most recently completed test. Also updates the 
//  returns the program state to the main menu or to display results
//
// Inputs : 
//		PB_INPUT_TYPE pb_type: pushbutton input type
//
// Outputs : none
//
//**************************************************************************
void discard_test_results(PB_INPUT_TYPE pb_type)
{	
	/* OK PB press to discard results & return to main menu */
	if (pb_type == OK)
	{
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	/* BACK PB press to go back to displaying current results*/
	else if (pb_type == BACK)
	{
		TEST_CURRENT_STATE = DISPLAY_RESULTS;	
		display_test_results(NONE, LOADED_battery_voltages, UNLOADED_battery_voltages);
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
// Inputs :
//		PB_INPUT_TYPE pb_type: pushbutton input type
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
		TEST_CURRENT_STATE = DISPLAY_RESULTS;
		display_test_results(NONE, LOADED_battery_voltages, UNLOADED_battery_voltages);
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
//***************************************************************************
//
// Function Name : "scroll_previous_entries"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function allows the user to scroll through the entries for all 
//  previously saved tests. Responds differently to OK and BACK 
//  PB presses depending on program state. 
//
// Inputs :
//		PB_INPUT_TYPE pb_type: pushbutton input type
//
// Outputs : none
//
//**************************************************************************
void scroll_previous_entries(PB_INPUT_TYPE pb_type)
{
	switch (pb_type)
	{
		case UP: 
			// Scroll up through quad pack entries
			move_cursor_up();	// update cursor & update quad_pack_entry
			display_quad_pack_entries();
			break;
		case DOWN: 
			// Scroll down through quad pack entries
			move_cursor_down();	// update cursor & update quad_pack_entry	
			display_quad_pack_entries();
			break;
		case OK:
			/* If in test state, OK overwrites previous data with current data */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{ 
				TEST_CURRENT_STATE = OVERWRITE_RESULTS;
				overwrite_previous_results(NONE);
			}
			/* If in viewing history state, OK displays results of entry pointed to by cursor */
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				/* Read test result from EEPROM and display menu for viewing its results */
				eeprom_read_block(&current_test_result, &test_results_history_eeprom[quad_pack_entry], sizeof(test_result));
				VIEW_HISTORY_CURRENT_STATE = SCROLL_QUADPACK_DATA_H;
				scroll_test_result_menu(NONE, current_test_result);
			}
			break;
		case BACK:
			/* If in test state, BACK returns to displaying test results */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{			
				TEST_CURRENT_STATE = SCROLL_QUADPACK_DATA_T;
				scroll_test_result_menu(NONE, current_test_result);				
			}
			/* If viewing history, BACK returns to main menu */
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				cursor = 1;		// Initialize cursor to line 1
				quad_pack_entry = 0;	// Initialize quad pack entry to quad pack 1
				LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
				display_main_menu();
			}
			break;			
		default:
			display_quad_pack_entries();
			break;
	}	
}
//***************************************************************************
//
// Function Name : "overwrite_previous_results"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays a message asking if the user would like to 
//  overwrite the results of a previous test and replace it with the results 
//  of the most recently completed test. 
//
// Inputs :
//		PB_INPUT_TYPE pb_type: pushbutton input type
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
// Function Name : "move_cursor_up"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function moves the display cursor up and decrements the previous
//  quad pack entry that it is pointing to.
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void move_cursor_up(void)
{
	/* Only 2 lines in main menu */
	if (LOCAL_INTERFACE_CURRENT_STATE == MAIN_MENU_STATE)
		if (cursor == 1)
			cursor = 2;
		else 
			cursor = 1;
	else
	{
		/* Only 4 lines on LCD, user cannot scroll up beyond line 1 */
		if (cursor != 1)
			cursor--;
	
		// Only 13 quad pack entries, entry rolls around to 13 if user scrolls up beyond 1
		if (quad_pack_entry == 0)	// row index for 2D array, 0 is index for 1st row
			quad_pack_entry = 12;	// row index for 2D array, 12 is index for 13th row
		else
			quad_pack_entry--;		
	}
}
//***************************************************************************
//
// Function Name : "move_cursor_down"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function moves the display cursor down and increments the previous
//  quad pack entry that it is pointing to.
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void move_cursor_down(void)
{
	/* Only 2 lines in main menu */
	if (LOCAL_INTERFACE_CURRENT_STATE == MAIN_MENU_STATE)
		if (cursor == 1)
			cursor = 2;
		else
			cursor = 1;
	else	
	{	
		/* Only 4 lines on LCD, user cannot scroll down beyond line 4 */
		if (cursor != 4)
			cursor++;
	
		// Only 13 quad pack entries, entry rolls around to 1 if user scrolls down beyond 13
		if (quad_pack_entry == 12)	// row index for 2D array, 12 is index for 13th row
			quad_pack_entry = 0;	// row index for 2D array, 0 is index for 1st row
		else
			quad_pack_entry++;		
	}
}
//***************************************************************************
//
// Function Name : "display_main_menu"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays the main menu screen
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void display_main_menu(void)
{
	clear_lcd();

	/* Main menu strings */
	sprintf(dsp_buff[0], "Test                ");
	sprintf(dsp_buff[1], "View History        ");
	sprintf(dsp_buff[2], "Settings            ");

	/* Append cursor icon to end of string */
	for (uint8_t i = 0; i < 20; i++)
	{
		/* Only change buffer corresponding to cursor position */
		if (dsp_buff[cursor - 1] == ' ')
		{
			dsp_buff[i] = '<';
			dsp_buff[i+1] = '-';
		}	
	}
		
	update_lcd();
}
//***************************************************************************
//
// Function Name : "display_quad_pack_entries"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays a set of previous quad pack entries for the user
//  to select.
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void display_quad_pack_entries(void)
{
	clear_lcd();
	
	/* display cursor on same line as quad pack entry */
	if(quad_pack_entry + 1 >= 10)
		sprintf(dsp_buff[cursor - 1], "Quad pack %d   <-     ", quad_pack_entry + 1);
	else
		sprintf(dsp_buff[cursor - 1], "Quad pack %d    <-     ", quad_pack_entry + 1);
	
	uint8_t entries_above_cursor = cursor - 1;	// number of quad pack entries to be displayed above the cursor line
	uint8_t entries_below_cursor = 4 - cursor;	// number of quad pack entries to be displayed below the cursor line
	int8_t quad_pack_display;	// quad_pack_entry number that will appear on the current line of the display
	
	/* Start on line 4 and display all entries BELOW the entry pointed to by the cursor */
	while (entries_below_cursor != 0)
	{
		quad_pack_display = quad_pack_entry + entries_below_cursor + 1;	// quad pack entry number on each line BELOW cursor
		
		if (quad_pack_display < 1)
			quad_pack_display += 13;	// If value is <= 0, add 13 to ensure that display number 'rolls over' circularly
		if (quad_pack_display > 13)
			quad_pack_display -= 13;	// If value is >= 14, subtract 13 to ensure that display number 'rolls over' circularly
		
		if(quad_pack_display >= 10)
			sprintf(dsp_buff[cursor + entries_below_cursor - 1], "Quad pack %d        ", quad_pack_display);
		else
			sprintf(dsp_buff[cursor + entries_below_cursor - 1], "Quad pack %d         ", quad_pack_display);
		entries_below_cursor--;	// move up 1 line until the cursor line is reached
	}
	
	/* Start on line 1 and display all entries ABOVE the entry pointed to by the cursor */
	while (entries_above_cursor != 0)
	{
		quad_pack_display = quad_pack_entry - entries_above_cursor + 1;	// quad pack entry number on each line ABOVE cursor
		
		if (quad_pack_display < 1)
			quad_pack_display += 13;	// If value is <= 0, add 13 to ensure that display number 'rolls over' circularly
		if (quad_pack_display > 13)
			quad_pack_display -= 13;	// If value is >= 14, subtract 13 to ensure that display number 'rolls over' circularly
		
		if(quad_pack_display >= 10)
			sprintf(dsp_buff[cursor - entries_above_cursor - 1], "Quad pack %d        ", quad_pack_display);
		else
			sprintf(dsp_buff[cursor - entries_above_cursor - 1], "Quad pack %d         ", quad_pack_display);
		entries_above_cursor--;	// move down 1 line until the cursor line is reached
	}	
	
	update_lcd();	
}
//***************************************************************************
//
// Function Name : "buzzer_ON"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function turns the buzzer ON to make a beep sound
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void buzzer_ON(void)
{
	VPORTA.DIR |= PIN6_bm;  // Set PC1 as output
	VPORTA.OUT |= PIN6_bm;	    // make PC1 output 1, make buzzer beep
}
//***************************************************************************
//
// Function Name : "buzzer_ON"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function turns the buzzer OFF to stop the beep sound
// Inputs : none
//
// Outputs : none
//	
void buzzer_OFF(void)
{
	VPORTA.OUT &= !PIN6_bm;		// make PA6 output 0, make buzzer quiet
}

//***************************************************************************
//
// Function Name : "PB_init"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function configures push button IO pins
//
// Inputs :
//
// Outputs : none
//
//**************************************************************************
void PB_init(void)
{
	/* Configure push button IO pins */
	VPORTA_DIR &= ~(PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm);
		
	PORTA_PIN2CTRL = (PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm); //enable interrupts on PA2, and the internal pull-up resistor
	PORTA_PIN3CTRL = (PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm); //enable interrupts on PA3, and the internal pull-up resistor
	PORTA_PIN4CTRL = (PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm); //enable interrupts on PA4, and the internal pull-up resistor
	PORTA_PIN5CTRL = (PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm); //enable interrupts on PA5, and the internal pull-up resistor
}


