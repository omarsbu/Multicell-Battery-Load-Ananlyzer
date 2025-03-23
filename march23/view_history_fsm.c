#include "main.h"

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
		case SCROLL_TEST_RESULT_MENU_H:
			scroll_test_result_menu(PB_PRESS, current_test_result);
			break;
		case VOLTAGE_READINGS_H:
			if (PB_PRESS == BACK)
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
			else
				display_voltage_readings(current_test_result);
			break;
		case HEALTH_RATINGS_H:
			if (PB_PRESS == BACK)
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
			else
				display_health_ratings(current_test_result);
			break;
		case TEST_CONDITIONS_H:
			if (PB_PRESS == BACK)
				VIEW_HISTORY_CURRENT_STATE = SCROLL_PREVIOUS_RESULTS;
			else
				display_test_conditions(current_test_result);
			break;
		case DISCARD_RESULTS_H:
			discard_test_results(PB_PRESS);
			break;
		default:
			break;
	}
	return;
}