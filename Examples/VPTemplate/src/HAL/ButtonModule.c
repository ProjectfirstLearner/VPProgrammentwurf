/******************************************************************************
 * @file ButtonModule.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the Button Module
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

#include "HardwareConfig.h"
#include "ButtonModule.h"


/***** PRIVATE CONSTANTS *****************************************************/
#define BUTTON_DEBOUNCE_TICKS	5u
#define BUTTON_COUNT			3u

/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/
typedef struct
{
	Button_Status_t stableState;
	Button_Status_t lastRawState;
	uint8_t stableCounter;
} ButtonDebounceData_t;

/***** PRIVATE PROTOTYPES ****************************************************/
static Button_Status_t buttonGetRawButtonStatus(Button_t button);
static uint32_t buttonToIndex(Button_t button);

/***** PRIVATE VARIABLES *****************************************************/
static ButtonDebounceData_t gButtonDebounce[BUTTON_COUNT];

/***** PUBLIC FUNCTIONS ******************************************************/

int32_t buttonInitialize()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();


	/* Configure GPIO pin : B1 */
	GPIO_InitStruct.Pin 	= B1_PIN;
	GPIO_InitStruct.Mode 	= GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_PORT, &GPIO_InitStruct);

	/* Configure GPIO pin : SW1 */
	GPIO_InitStruct.Pin = SW1_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(SW1_GPIO_PORT, &GPIO_InitStruct);

	/* Configure GPIO pin : SW2 */
	GPIO_InitStruct.Pin = SW2_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(SW2_GPIO_PORT, &GPIO_InitStruct);

	/* Initialize debounce states with current raw button values */
	gButtonDebounce[BTN_SW1].stableState	= buttonGetRawButtonStatus(BTN_SW1);
	gButtonDebounce[BTN_SW1].lastRawState	= gButtonDebounce[BTN_SW1].stableState;
	gButtonDebounce[BTN_SW1].stableCounter	= 0u;

	gButtonDebounce[BTN_SW2].stableState	= buttonGetRawButtonStatus(BTN_SW2);
	gButtonDebounce[BTN_SW2].lastRawState	= gButtonDebounce[BTN_SW2].stableState;
	gButtonDebounce[BTN_SW2].stableCounter	= 0u;

	gButtonDebounce[BTN_B1].stableState		= buttonGetRawButtonStatus(BTN_B1);
	gButtonDebounce[BTN_B1].lastRawState	= gButtonDebounce[BTN_B1].stableState;
	gButtonDebounce[BTN_B1].stableCounter	= 0u;

	return BUTTON_ERR_OK;
}

void buttonCyclic10ms()
{
	uint32_t index = 0u;
	Button_t button = BTN_SW1;
	Button_Status_t rawState = BUTTON_RELEASED;

	for(index = 0u; index < BUTTON_COUNT; index++)
	{
		button = (Button_t)index;
		rawState = buttonGetRawButtonStatus(button);

		if(rawState == gButtonDebounce[index].lastRawState)
		{
			if(gButtonDebounce[index].stableCounter < BUTTON_DEBOUNCE_TICKS)
			{
				gButtonDebounce[index].stableCounter++;
			}
			else
			{
				/* nothing to do */
			}
		}
		else
		{
			gButtonDebounce[index].lastRawState = rawState;
			gButtonDebounce[index].stableCounter = 1u;
		}

		if ((gButtonDebounce[index].stableCounter >= BUTTON_DEBOUNCE_TICKS) &&
			 (gButtonDebounce[index].stableState != rawState))
		{
			gButtonDebounce[index].stableState = rawState;
		}
	}
}

Button_Status_t buttonGetButtonStatus(Button_t button)
{
	uint32_t index = buttonToIndex(button);
	return gButtonDebounce[index].stableState;
}

/***** PRIVATE FUNCTIONS *****************************************************/

static Button_Status_t buttonGetRawButtonStatus(Button_t button)
{
    GPIO_PinState gpioStatus = GPIO_PIN_RESET;
    Button_Status_t buttonStatus = BUTTON_RELEASED;

    switch (button)
    {
        // Read the Ground Floor button (internal pull-up --> GPIO=1 ==> Button released)
        case BTN_SW1:
        {
            gpioStatus = HAL_GPIO_ReadPin(SW1_GPIO_PORT, SW1_PIN);
            if (gpioStatus == GPIO_PIN_RESET)
                buttonStatus = BUTTON_PRESSED;
            else
                buttonStatus = BUTTON_RELEASED;
        }
        break;

        // Read the First Floor button (internal pull-up --> GPIO=1 ==> Button released)
        case BTN_SW2:
        {
            gpioStatus = HAL_GPIO_ReadPin(SW2_GPIO_PORT, SW2_PIN);
            if (gpioStatus == GPIO_PIN_RESET)
                buttonStatus = BUTTON_PRESSED;
            else
                buttonStatus = BUTTON_RELEASED;
        }
        break;

        // Read the First Floor button (internal pull-up --> GPIO=1 ==> Button released)
		case BTN_B1:
		{
			gpioStatus = HAL_GPIO_ReadPin(B1_GPIO_PORT, B1_PIN);
			if (gpioStatus == GPIO_PIN_SET)
				buttonStatus = BUTTON_PRESSED;
			else
				buttonStatus = BUTTON_RELEASED;
		}
		break;
    }

    return buttonStatus;
}

static uint32_t buttonToIndex(Button_t button)
{
	uint32_t index = 0u;

	switch(button)
	{
		case BTN_SW1:
			index = 0u;
			break;

		case BTN_SW2:
			index = 1u;
			break;

		case BTN_B1:
			index = 2u;
			break;

		default:
			index = 0u;
			break;
	}

	return index;
}
