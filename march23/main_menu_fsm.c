#include "main.h"

//***************************************************************************
//
// Function Name : "main_menu_fsm"
// Target MCU : AVR128DB48
// DESCRIPTION
// Main menu finite state machine. This finite state machine only has one 
//	state and based on which pushbutton was pressed, it either updates the 
//	cursor and its position on the display or it transfers control to the
//  test fsm, the view history fsm, or the settings fsm.
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
		/* OK pushbutton press -> transfer control to the selected fsm */
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
				SETTING_CURRENT_STATE = SCROLL_SETTINGS;	
				PB_PRESS = NONE;	// Clear pushbutton state before transitioning to new fsm
				settings_fsm();	// Enter view settings fsm			
			}
			break;
		/* BACK pushbutton press -> do nothing in main menu*/
		case BACK:
			asm volatile ("nop");	// do nothing
			break;
		/* UP pushbutton press -> Scroll up one line of main menu */
		case UP:
			move_cursor_up();
			display_main_menu();
			break;
		/* DOWN pushbutton press -> Scroll down one line of main menu */
		case DOWN:
			move_cursor_down();
			display_main_menu();
			break;
		/* Default action is the display main menu screen */
		default:
			display_main_menu();
			break;
	}

	return;
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
	/* Only 3 lines in main menu */
	if (LOCAL_INTERFACE_CURRENT_STATE == MAIN_MENU_STATE)
	{
		switch(cursor) 
		{
			case 1 : cursor = 3; break;	// cursor loops back around to line 3 when scrolling up past line 1
			case 2 : cursor = 1; break;
			case 3 : cursor = 2; break;
			default: cursor = 1; break;
		}
	}
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
	/* Only 3 lines in main menu */
	if (LOCAL_INTERFACE_CURRENT_STATE == MAIN_MENU_STATE)
	{
		switch(cursor)
		{
			case 1 : cursor = 2; break;
			case 2 : cursor = 3; break;
			case 3 : cursor = 1; break;	// cursor loops back around to line 1 when scrolling down past line 3
			default: cursor = 1; break;
		}
	}
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
			dsp_buff[cursor - 1][i] = '<';
			dsp_buff[cursor - 1][i+1] = '-';
			break;	// exit for loop after appending cursor;
		}	
	}
		
	update_lcd();
}