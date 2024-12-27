#include <avr/io.h>
#define F_CPU 4000000UL
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "main.h"

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
	/* Set pushbutton type to OK and enter local interface fsm*/
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
	/* Set pushbutton type to BACK and enter local interface fsm*/
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
	/* Set pushbutton type to UP and enter local interface fsm*/
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
	/* Set pushbutton type to DOWN and enter local interface fsm*/
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
		default:
			// return to main menu state
			LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
			main_menu_fsm();
			break;
	}
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
				// check whether to display error message or proceed with test fsm
				// this functIon will set TEST_CURRENT_STATE to either ERROR or TESTING
				is_battery_connected();	
				test_fsm();	// Enter test fsm
			}
			/* OK PB pressed while cursor was on 'View History' option */
			else if (cursor == 2) 
			{
				/* Update program state variables */
				LOCAL_INTERFACE_CURRENT_STATE = VIEW_HISTORY_STATE;
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
				view_history_fsm();	// Enter view history fsm
			}
			break;
		case BACK:	// do nothing
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
			asm volatile ("nop");	// do nothing
			break;
	}

	PB_PRESS = NONE;	// Clear pushbutton state after it is handled
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
			display_error_message();
			break;
		case TESTING:
			perform_test();
			break;
		case DISPLAY_RESULTS:
			display_test_results(LOADED_battery_voltages, UNLOADED_battery_voltages);
			break;
		case DISCARD_RESULTS:
			discard_test_results();
			break;
		case SAVE_CURRENT_RESULTS:
			save_test_results();
			break;
		case SCROLL_SAVE_ENTRIES:
			scroll_previous_entries();
			break;
		case OVERWRITE_RESULTS:
			overwrite_previous_results();
			break;
		default:
			break;
	}

	PB_PRESS = NONE;	// Clear pushbutton state after it is handled
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
//	TEST_FSM because it determines the initial state: ERROR or TESTING
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void is_battery_connected(void)
{
	/* Function sets the test state to ERROR or TESTING */
	ADC0.MUXPOS = 0x03; // ADC input pin AIN3, 13.2V input
	ADC0.MUXNEG = 0x40; // GND
	
	// Read total battery pack voltage
	float voltage = (float)(((battery_voltage_divider_ratios)*adc_vref)*(ADC_read()/2048));
	
	// If voltage < 100mV, there is no battery connection move to ERROR state, otherwise procede with test
	if (voltage < 0.1)
		TEST_FSM_STATES = ERROR;	// Move to ERROR state
	else
		TEST_FSM_STATES = TESTING;	// Proceed with TEST

	PB_PRESS = NONE;	// clear PB press
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
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void display_error_message (void)
{
	/* Return to main menu if OK or BACK PB is pressed, display ERROR message otherwise */
	if (PB_PRESS = OK || PB_PRESS = BACK)
	{		
		PB_PRESS = NONE;	// clear PB press
		cursor = 1;		// Initialize cursor to line 1
		quad_pack_entry = 0;	// Initialize quad pack entry to quad pack 1
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	else
	{
		sprintf(dsp_buff[0], "Failed! Ensure");
		sprintf(dsp_buff[1], "Proper Connection");
		sprintf(dsp_buff[2], "Press OK or BACK");
		sprintf(dsp_buff[3], "to Continue");
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
	
	/* Tell user to rotate knob of carbon pile until beep indicates 500A... */
	clear_lcd();
	sprintf(dsp_buff[0], "Rotate Knob until"); 
	sprintf(dsp_buff[1], "beeping sound is");
	sprintf(dsp_buff[2], "heard");
	update_lcd();
	
	// Turn ON fan to prevent overheating
	set_Fan_PWM(75);
	
	// Read load current and wait until it hits 500A +/- 20A error
	load_current_amps = load_current_Read();
	while (load_current_amps < 480 && load_current_amps < 700)	// infinite loop until current reaches 500A
		load_current_amps = load_current_Read();
	
	// read voltage of each cell and store in array once load current reaches 500A
	read_LOADED_battery_voltages();

	/* Tell user to turn off carbon pile load... */
	clear_lcd();
	sprintf(dsp_buff[0], "Test complete...");
	sprintf(dsp_buff[1], "Rotate Knob until");
	sprintf(dsp_buff[2], "beeping stops");
	update_lcd();
	
	/* Make buzzer beep until current is below 50A */
	VPORTC.DIR |= PIN1_bm; // Set PC1 as output, make buzzer beep
	
	while (load_current_amps > 50)
	{
		load_current_amps = load_current_Read();
		
		VPORTC.OUT |= 0x02;	    // make PC1 output 1, make buzzer beep
		_delay_ms(1000);	    // wait 1 seconds
		VPORTC.OUT &= 0xFD;		// make PC1 output 0, make buzzer quiet
		_delay_ms(1000);		// wait 1 seconds
	}
	
	set_Fan_PWM(0);	// Turn fan OFF

	// Proceed to next state -> display test results	
	PB_PRESS = NONE;	// clear PB press
	TEST_FSM_STATES = DISPLAY_RESULTS;	
	display_test_results(LOADED_battery_voltages, UNLOADED_battery_voltages);
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
// Inputs : 2 4-element integer arrays
//
// Outputs : none
//
//**************************************************************************
void display_test_results(uint8_t LOADED_results[4], uint8_t UNLOADED_results[4])
{
	/* OK PB press to save results, BACK PB press to discard results*/
	if (PB_PRESS == OK)
	{
		PB_PRESS = NONE;	// clear PB press
		// Proceed to next state -> save test results
		TEST_FSM_STATES = SAVE_CURRENT_RESULTS;
		save_test_results();
	}
	else if (PB_PRESS == BACK)
	{
		PB_PRESS = NONE;	// clear PB press
		// Proceed to next state -> discard test results
		TEST_FSM_STATES = DISCARD_RESULTS;
		discard_test_results();
	}
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
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void discard_test_results(void)
{	
	/* OK PB press to discard results & return to main menu */
	if (PB_PRESS == OK)
	{
		PB_PRESS = NONE;	// clear PB press
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	/* BACK PB press to go back to displaying current results*/
	else if (PB_PRESS == BACK)
	{
		PB_PRESS = NONE;	// clear PB press
		TEST_FSM_STATES = DISPLAY_RESULTS;	
		display_test_results(LOADED_battery_voltages, UNLOADED_battery_voltages);
	}
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Press OK to ");
		sprintf(dsp_buff[1], "permanently discard");
		sprintf(dsp_buff[2], "test results, press");
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
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void save_test_results(void)
{
	/* OK PB press to show entries to save results */
	if (PB_PRESS == OK)
	{
		PB_PRESS = NONE;	// clear PB press	
		cursor = 1;		// Initialize cursor to line 1
		quad_pack_entry = 0;	// Initialize quad pack entry to quad pack 1
		TEST_FSM_STATES = SCROLL_SAVE_ENTRIES;
		scroll_previous_entries();
	}	
	/* BACK PB press to go back to displaying current results*/
	else if (PB_PRESS == BACK)
	{
		PB_PRESS = NONE;	// clear PB press
		TEST_FSM_STATES = DISPLAY_RESULTS;
		display_test_results(LOADED_battery_voltages, UNLOADED_battery_voltages);
	}
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Save results?");
		sprintf(dsp_buff[1], "Press OK");
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
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void scroll_previous_entries(void)
{
	switch (PB_PRESS)
	{
		case UP: 
			// Scroll up through quad pack entries
			move_cursor_up();	
			display_quad_pack_entries();
			break;
		case DOWN: 
			// Scroll down through quad pack entries
			move_cursor_down();	
			display_quad_pack_entries();
			break;
		case OK:
			/* If in test state, OK overwrites previous data with current data */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{ 
				PB_PRESS = NONE;	// clear PB press
				TEST_FSM_STATES = OVERWRITE_RESULTS;
				overwrite_previous_results();
			}
			/* If in viewing history state, OK displays results of entry pointed to by cursor */
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				PB_PRESS = NONE;	// clear PB press
				VIEW_HISTORY_CURRENT_STATE = DISPLAY_PREVIOUS_RESULTS;
				display_test_results(HISTORY_LOADED_battery_voltages[quad_pack_entry], HISTORY_UNLOADED_battery_voltages[quad_pack_entry]);
			}
			break;
		case BACK:
			/* If in test state, BACK returns to displaying test results */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{
				PB_PRESS = NONE;	// clear PB press
				TEST_FSM_STATES = DISPLAY_RESULTS;
				display_test_results(LOADED_battery_voltages, UNLOADED_battery_voltages);
			}
			/* If viewing history, BACK returns to main menu */
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				PB_PRESS = NONE;	// clear PB press
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
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void overwrite_previous_results(void)
{
	/* OK pushbutton press to overwrite results and return to main menu */
	if (PB_PRESS == OK)
	{
		/* Copy current test results into history matrix rows pointed to by quad_pack_entry */		
		for (uint8_t i = 0; i < 4; i++)
		{
			// Replace values from previous test with values from new test 
			HISTORY_UNLOADED_battery_voltages[quad_pack_entry][i] = UNLOADED_battery_voltages[i];
			HISTORY_LOADED_battery_voltages[quad_pack_entry][i] = LOADED_battery_voltages[i];
		}
		
		/* Return to main menu */
		PB_PRESS = NONE;	// clear PB press
		cursor = 1;		// Initialize cursor to line 1
		quad_pack_entry = 0;	// Initialize quad pack entry to 1. Row index to 2D array, 0 is index to 1st entry
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
    /* BACK pushbutton press to go back to displaying current results */
	else if (PB_PRESS == BACK)
	{
		PB_PRESS = NONE;	// clear PB press
		TEST_FSM_STATES = DISPLAY_RESULTS;	
		display_test_results(LOADED_battery_voltages, UNLOADED_battery_voltages);
	}
	else
	{
		clear_lcd();
		sprintf(dsp_buff[0], "Press OK to overwrite");
		sprintf(dsp_buff[1], "previous results");
		sprintf(dsp_buff[2], "Press BACK to");
		sprintf(dsp_buff[3], "view current results");
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
	// Only 2 lines in main menu
	if (LOCAL_INTERFACE_CURRENT_STATE == MAIN_MENU_STATE)
		if (cursor == 1)
			cursor = 2;
		else 
			cursor = 1;
	else
	{
		// Only 4 lines on LCD, user cannot scroll up beyond line 1
		if (cursor =! 1)
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
	// Only 2 lines in main menu
	if (LOCAL_INTERFACE_CURRENT_STATE == MAIN_MENU_STATE)
		if (cursor == 1)
			cursor = 2;
		else
			cursor = 1;
	else	
	{	
		// Only 4 lines on LCD, user cannot scroll down beyond line 4
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
	sprintf(dsp_buff[0], "Test");
	sprintf(dsp_buff[1], "View History");
	sprintf(dsp_buff[cursor - 1], " <-");	// Append cursor to correct line
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
	
	sprintf(dsp_buff[cursor - 1], "Quad pack %d <-", quad_pack_entry + 1);	// display cursor on same line as quad pack entry
	
	uint8_t entries_above_cursor = cursor - 1;	// number of quad pack entries to be displayed above the cursor line
	uint8_t entries_below_cursor = 4 - cursor;	// number of quad pack entries to be displayed below the cursor line
	uint8_t quad_pack_display;	// quad_pack_entry number that will appear on the current line of the display
	
	/* Start on line 4 and display all entries BELOW the entry pointed to by the cursor */
	while (entries_below_cursor != 0)
	{
		quad_pack_display = quad_pack_entry + entries_below_cursor + 1	// quad pack entry number on each line BELOW cursor
		
		if (quad_pack_display < 1)
			quad_pack_display += 13;	// If value is < 0, add 13 to ensure that display number 'rolls over' circularly
		
		sprintf(dsp_buff[cursor + entries_below_cursor - 1], "Quad pack %d", quad_pack_display);
		entries_below_cursor--;	// move up 1 line until the cursor line is reached
	}
	
	/* Start on line 1 and display all entries ABOVE the entry pointed to by the cursor */
	while (entries_above_cursor != 0)
	{
		quad_pack_display = quad_pack_entry - entries_above_cursor + 1	// quad pack entry number on each line ABOVE cursor
		
		if (quad_pack_display < 1)
			quad_pack_display += 13;	// If value is < 0, add 13 to ensure that display number 'rolls over' circularly
		
		sprintf(dsp_buff[cursor - entries_above_cursor - 1], "Quad pack %d", quad_pack_display);
		entries_above_cursor--;	// move down 1 line until the cursor is reached
	}	
	
	update_lcd();	
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
			scroll_previous_entries();
			break;
		case DISPLAY_PREVIOUS_RESULTS:
			display_test_results(HISTORY_LOADED_battery_voltages[quad_pack_entry], HISTORY_UNLOADED_battery_voltages[quad_pack_entry]);
			break;
		default:
			break;
	}

	PB_PRESS = NONE;	// Clear pushbutton state after it is handled
	return;
}
