/******************************************************************************
 * @file StackMonitor.c
 *
 * @brief Implementation of the stack monitor.
 *
 * This module evaluates the stack fill pattern written by the startup code.
 * The startup fills the whole stack with a known marker and places a dedicated
 * end marker at the lowest stack address.
 *
 * The implementation uses:
 * - _initial_stack_pointer from the linker script
 * - _size_of_stack from the linker script
 * - __get_MSP() to read the current main stack pointer
 *
 * Note:
 * One 32-bit word at the lowest stack address is reserved for the end marker.
 * Therefore, the usable stack size for application code is:
 *
 *   usable_stack_size = total_stack_size - sizeof(uint32_t)
 *
 ******************************************************************************/

/***** INCLUDES **************************************************************/
#include "StackMonitor.h"
#include "stm32g4xx.h"
#include <stdbool.h>

#include <stddef.h>
#include <stdint.h>

/***** PRIVATE CONSTANTS *****************************************************/

/* Must match the values used in startup_app.s */
#define STACK_FILL_MARKER      (0x0ABADD1EU)
#define STACK_END_MARKER       (0x0ABADA55U)

/***** PRIVATE LINKER SYMBOLS ************************************************/

/*
 * These symbols are provided by the linker script.
 *
 * Important:
 * For linker symbols, the symbol value is represented by the symbol address.
 * Therefore, the code uses '&symbol' and casts it to an integer address.
 */
extern uint32_t _initial_stack_pointer;
extern uint32_t _size_of_stack;

/***** PRIVATE FUNCTION PROTOTYPES *******************************************/

static uintptr_t stackGetInitialPointer(void);
static uintptr_t stackGetTotalSizeBytes(void);
static uintptr_t stackGetBottomAddress(void);
static uintptr_t stackGetTopUsableAddress(void);
static uintptr_t stackGetFirstMarkerAddress(void);
static uintptr_t stackGetUsableSizeBytes(void);

/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Returns the initial stack pointer defined by the linker.
 *
 * Example from linker:
 * _initial_stack_pointer = ORIGIN(STACK) + LENGTH(STACK) - 4
 *
 * @return Initial stack pointer address.
 */
static uintptr_t stackGetInitialPointer(void)
{
    return (uintptr_t)&_initial_stack_pointer;
}

/**
 * @brief Returns the configured total stack size in bytes.
 *
 * Example from linker:
 * _size_of_stack = 0x1000
 *
 * @return Total stack size in bytes.
 */
static uintptr_t stackGetTotalSizeBytes(void)
{
    return (uintptr_t)&_size_of_stack;
}

/**
 * @brief Returns the lowest address of the stack area.
 *
 * Since the linker provides the initial stack pointer and the total stack size,
 * the bottom address can be reconstructed as:
 *
 * bottom = initial_stack_pointer - total_stack_size + 4
 *
 * The +4 is needed because the initial stack pointer already points to the
 * highest usable stack word, not one word above the stack.
 *
 * @return Lowest stack address.
 */
static uintptr_t stackGetBottomAddress(void)
{
    return stackGetInitialPointer() - stackGetTotalSizeBytes() + sizeof(uint32_t);
}

/**
 * @brief Returns the highest usable stack address.
 *
 * This is the address contained in _initial_stack_pointer.
 *
 * @return Highest usable stack address.
 */
static uintptr_t stackGetTopUsableAddress(void)
{
    return stackGetInitialPointer();
}

/**
 * @brief Returns the address of the first fill marker word.
 *
 * The very first word at the bottom of the stack contains the end marker.
 * The fill marker starts one word above it.
 *
 * @return Address of first fill marker word.
 */
static uintptr_t stackGetFirstMarkerAddress(void)
{
    return stackGetBottomAddress() + sizeof(uint32_t);
}

/**
 * @brief Returns the usable stack size in bytes.
 *
 * One 32-bit word is occupied by the end marker and is therefore not counted
 * as usable stack space.
 *
 * @return Usable stack size in bytes.
 */
static uintptr_t stackGetUsableSizeBytes(void)
{
    return stackGetTotalSizeBytes() - sizeof(uint32_t);
}

/***** PUBLIC FUNCTIONS ******************************************************/

int32_t getFreeBytes(void)
{
    uintptr_t currentAddress = stackGetFirstMarkerAddress();
    uintptr_t topAddress     = stackGetTopUsableAddress();
    int32_t freeBytes        = 0;

    /* Scan from low to high addresses through the marker area.
     * As long as the marker is still present, the stack has never used
     * this word.
     */
    while (currentAddress <= topAddress)
    {
        uint32_t currentWord = *((volatile uint32_t*)currentAddress);

        if (currentWord != STACK_FILL_MARKER)
        {
            break;
        }

        freeBytes += (int32_t)sizeof(uint32_t);
        currentAddress += sizeof(uint32_t);
    }

    return freeBytes;
}

int32_t getUsedBytes(void)
{
    int32_t usableBytes = (int32_t)stackGetUsableSizeBytes();
    int32_t freeBytes   = getFreeBytes();
    int32_t usedBytes   = usableBytes - freeBytes;

    if (usedBytes < 0)
    {
        usedBytes = 0;
    }

    return usedBytes;
}

int32_t getUsage(void)
{
    int32_t usableBytes = (int32_t)stackGetUsableSizeBytes();
    int32_t usedBytes   = getUsedBytes();

    if (usableBytes <= 0)
    {
        return 0;
    }

    int32_t usagePercent = (usedBytes * 100) / usableBytes;

    if (usagePercent > 100)
    {
        usagePercent = 100;
    }

    return usagePercent;
}

bool isCorrupted(void)
{
    uintptr_t bottomAddress = stackGetBottomAddress();
    uintptr_t mspAddress    = (uintptr_t)__get_MSP();

    uint32_t endMarkerValue = *((volatile uint32_t*)bottomAddress);

    /* Condition 1:
     * The dedicated end marker at the lowest stack address must still exist.
     * If not, the stack already overran its lower boundary.
     */
    if (endMarkerValue != STACK_END_MARKER)
    {
        return true;
    }

    /* Condition 2:
     * The current MSP must stay above the end marker word.
     * If the MSP points into or below the protected word, the stack is no
     * longer in a valid area.
     */
    if (mspAddress <= bottomAddress)
    {
        return true;
    }

    return false;
}
