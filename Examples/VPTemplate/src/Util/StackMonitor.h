/******************************************************************************
 * @file StackMonitor.h
 *
 * @brief Public interface of the stack monitor.
 *
 * The stack monitor evaluates the stack area that was initialized in the
 * startup code with a fill marker. The implementation assumes:
 *
 * - the stack is filled with 0x0ABADD1E during startup
 * - the lowest stack address contains the end marker 0x0ABADA55
 * - the Cortex-M4 uses a full-descending stack
 *
 * The values returned by this module represent the maximum stack consumption
 * since startup, not only the instantaneous stack usage.
 *
 ******************************************************************************/

#ifndef STACK_MONITOR_H_
#define STACK_MONITOR_H_

/***** INCLUDES **************************************************************/
#include <stdbool.h>
#include <stdint.h>

/***** PUBLIC FUNCTION PROTOTYPES ********************************************/

/**
 * @brief Returns the number of still untouched stack bytes.
 *
 * The function scans the stack area from the lower stack boundary upwards
 * until the first word is found that no longer contains the fill marker.
 *
 * @return Number of free stack bytes.
 */
int32_t getFreeBytes(void);

/**
 * @brief Returns the number of bytes that have been used on the stack.
 *
 * This value is derived from:
 * used = usable_stack_size - free_stack_size
 *
 * @return Number of used stack bytes.
 */
int32_t getUsedBytes(void);

/**
 * @brief Returns the stack usage in percent.
 *
 * The returned value is an integer percentage in the range 0..100.
 *
 * @return Stack usage in percent.
 */
int32_t getUsage(void);

/**
 * @brief Checks whether the stack protection markers were corrupted.
 *
 * Corruption is assumed if:
 * - the end marker at the lowest stack address was overwritten, or
 * - the current main stack pointer moved into or below the protected end area
 *
 * @retval true  Stack corruption detected
 * @retval false No corruption detected
 */
bool isCorrupted(void);

#endif /* STACK_MONITOR_H_ */
