

#if 0
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/spi1.h"
#include "mcc_generated_files/memory/flash.h"
#include "mcc_generated_files/interrupt_manager.h"
#endif


#include <stdint.h>
#include <stdbool.h>

extern void flash_led(uint8_t num);

#if 0
	// include in main to use
    INTCON2bits.AIVTEN = 1;
    PIN_MANAGER_enableInterrupt_On_Change();
    INTERRUPT_GlobalEnable();
#endif

void __attribute__((section(".newaivt"), interrupt, no_auto_psv, context, used)) _UserCNCInterrupt(void)
{
	// test
	while(1)
	{
		flash_led(5);
		flash_led(1);
		flash_led(2);
	}

	return;
}

