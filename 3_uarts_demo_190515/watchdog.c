/*
 * watchdog.c
 *
 *  Created on: 2017Äê11ÔÂ7ÈÕ
 *      Author: pony
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/watchdog.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "drivers/buttons.h"

extern uint32_t g_ui32SysClock;;
void watchdog_ini(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

    //
    // Set the period of the watchdog timer to 1 second.
    //
    ROM_WatchdogReloadSet(WATCHDOG0_BASE, g_ui32SysClock*300);

    //
    // Enable reset generation from the watchdog timer.
    //
    ROM_WatchdogResetEnable(WATCHDOG0_BASE);

    //
    // Enable the watchdog timer.
    //
//    ROM_WatchdogEnable(WATCHDOG0_BASE);

}

