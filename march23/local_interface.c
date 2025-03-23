#include "main.h"

ISR(PORTA_PORT_vect)
{
	cli();

	if (VPORTA_INTFLAGS & PIN2_bm)
	{
		OK_ISR();
		VPORTA_INTFLAGS |= PIN2_bm;
	}
	else if(VPORTA_INTFLAGS & PIN3_bm)
	{
		BACK_ISR();
		VPORTA_INTFLAGS |= PIN3_bm;
	}
	else if(VPORTA_INTFLAGS & PIN4_bm)
	{
		UP_ISR();
		VPORTA_INTFLAGS |= PIN4_bm;
	}
	else if(VPORTA_INTFLAGS & PIN5_bm)
	{
		DOWN_ISR();
		VPORTA_INTFLAGS |= PIN5_bm;
	}
	
	_delay_ms(100);	// software debounce
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
//  fsm, the view history fsm, or the settings fsm. 
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
		/* Main menu fsm handles pushbutton press */
		case MAIN_MENU_STATE:
			main_menu_fsm(); 
			break;
		/* Test fsm handles pushbutton press */
		case TEST_STATE:		
			test_fsm();
			break;
		/* View history fsm handles pushbutton press */
		case VIEW_HISTORY_STATE:
			view_history_fsm();	
			break;
		/* Settings fsm handles pushbutton press */
		case SETTINGS_STATE:
			settings_fsm();
		/* Default state is main menu state */
		default:
			LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
			main_menu_fsm();
			break;
	}
	
	PB_PRESS = NONE;	// Clear pushbutton state after it is handled
	_delay_ms(200);		// Software debounce
	return;
}

//***************************************************************************
//
// Function Name : "discard_test_results"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays a message asking if the user would like to delete
//  the results of a quad pack test. When discarded all EEPROM data for the
//	test is replaced with 0's. Return to main menu after discarding results.	
//
// Inputs :
//		PB_INPUT_TYPE pb_type: pushbutton input type
//
// Outputs : none
//
//**************************************************************************
void discard_test_results(PB_INPUT_TYPE pb_type)
{
	/* OK PB press -> Discard results & return to main menu */
	if (pb_type == OK)
	{
		/* Erase current test result data */
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
		
		/* Erase old test data from EEPROM */
		if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE) {
			if (VIEW_HISTORY_CURRENT_STATE == DISCARD_RESULTS_H) {
				eeprom_update_block(&current_test_result, &test_results_history_eeprom[quad_pack_entry], sizeof(test_result));
			}
		}
		
		/* Return to main menu*/
		LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
		display_main_menu();
	}
	/* BACK PB press -> Go back to previous display state */
	else if (pb_type == BACK)
	{
		/* TESTING state -> Return to results menu to view result data */
		if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
		{
			TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;
			scroll_test_result_menu(NONE, current_test_result);
		}
		/* VIEW HISTORY state -> Return to scrolling through saved data */
		else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
		{
			VIEW_HISTORY_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_H;
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
		/* UP pushbutton press -> Scroll up through quad pack entries */
		case UP: 
			move_cursor_up();	// update cursor & update quad_pack_entry
			display_quad_pack_entries();
			break;
		/* DOWN pushbutton press -> Scroll down through quad pack entries */
		case DOWN: 
			move_cursor_down();	// update cursor & update quad_pack_entry	
			display_quad_pack_entries();
			break;
		/* OK pushbutton press -> Overwrite or display result data */
		case OK:
			/* TESTING state -> OK overwrites previous data with current data */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{ 
				TEST_CURRENT_STATE = OVERWRITE_RESULTS;
				overwrite_previous_results(NONE);
			}
			/* VIEW HISTORY state -> OK displays results of entry pointed to by cursor */
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				/* Read test result from EEPROM and display menu for viewing its results */
				eeprom_read_block(&current_test_result, &test_results_history_eeprom[quad_pack_entry], sizeof(test_result));
				VIEW_HISTORY_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_H;
				scroll_test_result_menu(NONE, current_test_result);
			}
			break;
		/* BACK pushbutton press -> Go back to previous display state */
		case BACK:
			/* TESTING state -> BACK returns to displaying test results */
			if (LOCAL_INTERFACE_CURRENT_STATE == TEST_STATE)
			{			
				TEST_CURRENT_STATE = SCROLL_TEST_RESULT_MENU_T;
				scroll_test_result_menu(NONE, current_test_result);				
			}
			/* VIEW HISTORY state -> BACK returns to main menu */
			else if (LOCAL_INTERFACE_CURRENT_STATE == VIEW_HISTORY_STATE)
			{
				cursor = 1;		// Initialize cursor to line 1
				quad_pack_entry = 0;	// Initialize quad pack entry to quad pack 1
				LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
				display_main_menu();
			}
			break;			
		/* Default action is to display the list of quad pack entries */
		default:
			display_quad_pack_entries();
			break;
	}	
}

//***************************************************************************
//
// Function Name : "display_quad_pack_entries"
// Target MCU : AVR128DB48
// DESCRIPTION
// This function displays a set of previous quad pack entries for the user
//  to select. The cursor position is on the same line as the quad pack 
//	entry the user is pointing to. There are 13 quad pack entries.
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
		sprintf(dsp_buff[cursor - 1], "Quad pack %d   <-     ", quad_pack_entry + 1);
	
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

