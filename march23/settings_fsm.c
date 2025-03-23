#include "main.h"

//***************************************************************************
//
// Function Name : "settings_fsm"
// Target MCU : AVR128DB48
// DESCRIPTION
// Settings finite state machine. This finite state machine has multiple 
//	states and it is responsible for displaying the settings menu and 
//	allowing the user to adjust the settings of the battery load analyzer.
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
settings_fsm(void)
{
	switch (SETTING_CURRENT_STATE)
	{
		/* Menu displaying settings with scrolling ability */
		case SCROLL_SETTINGS:
			scroll_settings_menu(PB_PRESS);
			break;
		/* Set Digits of load current in Amps individually */
		case LOAD_CURRENT_SETTINGS_SCREEN:
			adjust_load_current_settings(PB_PRESS);
			break;
		/* Set precision of voltage measurements in decimal places */
		case VOLTAGE_PRECISION_SETTINGS_SCREEN:
			adjust_voltage_precision_settings(PB_PRESS);
			break;
		/* Default action is to display the settings menu */
		default:
			SETTING_CURRENT_STATE = SCROLL_SETTINGS;
			display_settings_menu();
			break;
	}
	return;
}

//***************************************************************************
//
// Function Name : "scroll_settings_menu"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Function to handle pushbutton presses while in the settings menu.
//
// Inputs : PB_INPUT_TYPE pb_type : Pushbutton input identifier
//
// Outputs : none
//
//**************************************************************************
void scroll_settings_menu(PB_INPUT_TYPE pb_type)
{
	switch (PB_PRESS)
	{
		/* UP pushbutton press -> move cursor up 1 line */
		case UP:
			move_cursor_up();
			display_settings_menu();
			break;
		/* DOWN pushbutton press -> move cursor down 1 line */
		case DOWN:
			move_cursor_down();
			display_settings_menu();
			break;
		/* OK pushbutton press ->  Move to next screen or toggle selected setting */
		case OK:
			settings_menu_OK();
			break;
		/* BACK pushbutton press -> Return to main menu */
		case BACK:
			cursor = 1;
			quad_pack_entry = 0;
			LOCAL_INTERFACE_CURRENT_STATE = MAIN_MENU_STATE;
			display_main_menu();
			break;
		/* Default action is to display settings menu */
		default:
			display_settings_menu();
			break;
	}
}

//***************************************************************************
//
// Function Name : "settings_menu_OK"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Function to handle an OK pushbutton press while in the settings menu.
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void settings_menu_OK(void)
{
	switch(cursor)
	{
		/* LCD line 1: Toggle Test mode between manual and automated */
		case 1:
			testing_mode ^= 0x01;	// XOR operation to toggle lsb
			display_settings_menu();
			break;
		/* LCD line 2: New screen to set load current */
		case 2:
			SETTING_CURRENT_STATE = LOAD_CURRENT_SETTINGS_SCREEN;
			adjust_load_current_settings(NONE);
			break;
		/* LCD line 3: Set voltage precision in decimal places */
		case 3:
			SETTING_CURRENT_STATE = VOLTAGE_PRECISION_SETTINGS_SCREEN;
			adjust_voltage_precision_settings(NONE);
			break;
		/* LCD line 4: Set battery type */
			case 4:
			asm volatile ("nop");	// Feature unavailable in this version...do nothing
			break;
		/* Default action is to do nothing */
		default:
			asm volatile ("nop");
			break;
	}
}

//***************************************************************************
//
// Function Name : "adjust_voltage_precision_settings"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Function to handle a pushbutton press while the settings state is 
//	displaying the voltage precision setting in decimal places. Allows 
//	the user to modify the number of decimal places to use when making 
//	voltage measurements.
//
// Inputs : PB_INPUT_TYPE pb_type : Pushbutton input identifier
//
// Outputs : none
//
//**************************************************************************
void adjust_voltage_precision_settings(PB_INPUT_TYPE pb_type)
{
	/* OK pushbutton press -> increment precision, cannot exceed 3 decimal places */
	if (pb_type == OK)
	{
		/* Increment precision value, loop back to 0 after 3 decimal places */
		if (voltage_precision >= 3) {voltage_precision = 0;}
		else {voltage_precision++;}

		/* Update LCD */
		clear_lcd();
		sprintf(dsp_buff[0], "Voltage Precision:  ");
		sprintf(dsp_buff[1], "%u decimal points   ", voltage_precision);
		update_lcd();
	}
	/* BACK pushbutton press -> Return to settings menu */
	else if (pb_type == BACK)
	{
		SETTING_CURRENT_STATE =	SCROLL_SETTINGS;
		display_settings_menu();
	}
	else {asm volatile ("nop");}	// do nothing for UP/DOWN pushbutton presses
}

//***************************************************************************
//
// Function Name : "adjust_load_current_settings"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Function to handle a pushbutton press while the settings state is
//	displaying the load current setting in Amps. The first 3 lines of the 
//	display display the 3 bcd digits of the load current. The user can use
//  pushbuttons to toggle the bcd digits individually. Note that this is the
//	load current used during automated testing with the stepper motor.
//
// Inputs : PB_INPUT_TYPE pb_type : Pushbutton input identifier
//
// Outputs : none
//
//**************************************************************************
void adjust_load_current_settings(PB_INPUT_TYPE pb_type)
{
	switch(pb_type)
	{
		/* UP pushbutton press -> Move cursor up 1 line and update LCD */
		case UP:
			move_cursor_up();
			if (cursor == 4) {cursor = 3;}	// last line of this screen displays 'A', no cursor option available
			display_load_current_setting();
			break;
		/* DOWN pushbutton press -> Move cursor down 1 line and update LCD */
		case DOWN:
			move_cursor_down();
			if (cursor == 4) {cursor = 3;}	// last line of this screen displays 'A', no cursor option available
			display_load_current_setting();
			break;
		/* OK pushbutton press -> increment bcd value of load current */
		case OK:
			/* Increment 100s bcd digit, reset to 0 after 5 */
			if (cursor == 1)
			{
				if (current_setting_100_dig >= 5) {current_setting_100_dig = 0;}	// current cannot exceed 500A
				else {current_setting_100_dig++;}	// increment bcd value
			}
			/* Increment 10s bcd digit, reset to 0 after 9 */
			else if (cursor == 2)
			{
				if (current_setting_100_dig >= 5) {current_setting_10_dig = 0;}		// current cannot exceed 500A
				else if (current_setting_10_dig >= 9) {current_setting_10_dig = 0;}	// reset bcd value to 0
				else {current_setting_10_dig++;}	// increment bcd value
			}
			/* Increment 1s bcd digit, reset to 0 after 9 */
			else if (cursor == 3)
			{
				if (current_setting_100_dig >= 5) {current_setting_1_dig = 0;}		// current cannot exceed 500A
				else if (current_setting_1_dig >= 9) {current_setting_1_dig = 0;}	// reset bcd value to 0
				else {current_setting_1_dig++;}	// increment bcd value
			}
			else { asm volatile ("nop");}	// do nothing if cursor on line 4
		
			display_load_current_setting();	// update LCD screen with new load current value
			break;
		/* BACK pushbutton press -> return to settings menu */
		case BACK:
			/* bcd conversion to save current setting before returning to settings menu */
			current_setting = (100*current_setting_100_dig) + (10*current_setting_10_dig) + (1*current_setting_1_dig);
			SETTING_CURRENT_STATE =	SCROLL_SETTINGS;
			display_settings_menu();
			break;
		/* Default action is to display the load current digits */
		default:
			display_load_current_setting();
			break;
	}
}

//***************************************************************************
//
// Function Name : "display_load_current_setting"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Displays the 100s digit of the load current on line 1, the 10s digit on
//	line 2, and the 1s digit on line 3. The units are amps and it is 
//	displayed on line 4. 
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void display_load_current_setting(void)
{
	clear_lcd();
	sprintf(dsp_buff[0], "%u                  ", current_setting_100_dig);	// 100's bcd digit on 1st line
	sprintf(dsp_buff[1], "%u                  ", current_setting_10_dig);	// 10's bcd digit on 2nd line
	sprintf(dsp_buff[2], "%u                  ", current_setting_1_dig);	// 1's bcd digit on 3rd line
	sprintf(dsp_buff[3], "Amps                ");	// units on 4th line
	/* Append Cursor */
	dsp_buff[cursor - 1][19] = '<';
	dsp_buff[cursor - 1][20] = '-';
	update_lcd();
}

//***************************************************************************
//
// Function Name : "display_settings_menu"
// Target MCU : AVR128DB48
// DESCRIPTION
//	Displays the settings menu. 
//
// Inputs : none
//
// Outputs : none
//
//**************************************************************************
void display_settings_menu(void)
{
	clear_lcd();
	if (testing_mode == 0x00)	   {sprintf(dsp_buff[0], "Mode: Manual        ");}
	else if (testing_mode == 0x01) {sprintf(dsp_buff[0], "Mode: Automated     ");}
	sprintf(dsp_buff[1], "Load Current:%uA    " , current_setting);
	sprintf(dsp_buff[2], "Voltage DP:%u       ", voltage_precision);	// Decimal Point (DP)
	sprintf(dsp_buff[3], "Battery Type: Li-Ion");	// feature unavailable, default is Li-Ion....

	/* Append Cursor */
	dsp_buff[cursor - 1][19] = '<';
	dsp_buff[cursor - 1][20] = '-';
	update_lcd();
}