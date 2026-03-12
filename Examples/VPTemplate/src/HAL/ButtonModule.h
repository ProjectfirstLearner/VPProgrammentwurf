/******************************************************************************
 * @file ButtonModule.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for the Button Module
 *
 *
 *****************************************************************************/
#ifndef _BUTTON_MODULE_H_
#define _BUTTON_MODULE_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define BUTTON_ERR_OK      0            /* No error occured */

/***** TYPES *****************************************************************/

/**
 * @brief Enumeration of available Buttons and their usage
 *
 */
typedef enum _Button_t
{
    BTN_SW1,		/* Switch SW1	*/
    BTN_SW2,		/* Switch SW2	*/
    BTN_B1			/* Button B1	*/
} Button_t;

/**
 * @brief Enumeration for possible button states
 *
 */
typedef enum _Button_Status_t
{
    BUTTON_PRESSED,			/* Button is pressed  */
    BUTTON_RELEASED			/* Button is released */
} Button_Status_t;

/***** PROTOTYPES ************************************************************/

/**
 * @brief Initializes the GPIOs for the button inputs
 *
 * @return Returns BUTTON_ERR_OK if no error occurred
 */
int32_t buttonInitialize(void);

/**
 * @brief Processes the 10 ms debounce logic for all buttons
 *
 */
void buttonCyclic10ms(void);

/**
 * @brief Returns the debounced logical status of a button
 *
 * @param button Button to read the status from
 *
 * @return Returns the debounced button status
 */
Button_Status_t buttonGetButtonStatus(Button_t button);


#endif
