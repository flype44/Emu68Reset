/**********************************************************
 * Project:  Emu68Reset
 * Version:  0.1.1 (2024-01-22)
 * Author:   Philippe CARPENTIER (aka flype)
 * Target:   Amiga equipped with pistorm/emu68 accelerator card
 * Compiler: Amiga SAS/C 6.59
 **********************************************************/

/**********************************************************
 * HISTORY:
 * 0.1.0 (2024-01-21) First release
 * 0.1.1 (2024-01-22) Added Exec->Disable()
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "AsmFuncs.h"

/**********************************************************
 * Defines
 **********************************************************/

#define PM_WDOG_MAGIC   (0x5a000000)
#define PM_RSTC_FULLRST (0x00000020)
#define PM_RSTC ((volatile ULONG*)(0xf2000000 + 0x0010001c))
#define PM_RSTS ((volatile ULONG*)(0xf2000000 + 0x00100020))
#define PM_WDOG ((volatile ULONG*)(0xf2000000 + 0x00100024))

#define APP_VSTRING "$VER: Emu68Reset 0.1.1 (22.1.2024) [SAS/C 6.59] Philippe CARPENTIER"

#define TEMPLATE "HELP/S,DELAY/N"

/**********************************************************
 * Globals
 **********************************************************/

APTR DeviceTreeBase = NULL;

extern struct ExecBase   * SysBase;
extern struct DosLibrary * DOSBase;

static STRPTR verstring = APP_VSTRING;

typedef enum {
	OPT_HELP,
	OPT_DELAY,
	OPT_COUNT
} OPT_ARGS;

/**********************************************************
 ** 
 ** Entry point
 ** 
 **********************************************************/

ULONG main(ULONG argc, UBYTE *argv[])
{
	BOOL bCancel = FALSE;
	
	// Handle optional arguments
	
	if (argc > 1)
	{
		LONG opts[OPT_COUNT];
		struct RDArgs *rdargs;
		memset((char *)opts, NULL, sizeof(opts));
		
		if (rdargs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
		{
			if (opts[OPT_HELP] != NULL)
			{
				// HELP option
				
				bCancel = TRUE;
				
				printf(verstring + 6);
				printf("\nOptional: DELAY=<number of seconds> before the reboot.\n");
			}
			else if (opts[OPT_DELAY] != NULL)
			{
				// DELAY option
				
				ULONG nsecs = *(ULONG *)opts[OPT_DELAY];
				
				if (nsecs > 0)
				{
					Delay(nsecs * 50); // 50 system-ticks per second.
				}
			}
			
			FreeArgs(rdargs);
		}
		else
		{
			printf("Invalid arguments.\n");
		}
	}
	
	// Returns immediately if CANCELLED.
	
	if (bCancel)
	{
		return (RETURN_OK);
	}
	
	// Check if running on a PiStorm/Emu68 system.
	
	if (DeviceTreeBase = (struct Library *)OpenResource("devicetree.resource"))
	{
		ULONG rsts;
		
		// Disable the AmigaOS interrupts.
		
		Disable();
		
		// Kill the AmigaOS SysBase address.
		
		*((volatile ULONG*)(0x00000004)) = 0x00000000;
		
		// Trigger a restart by instructing the RPi GPU to boot from partition 0.
		
		rsts = asm_le32(*PM_RSTS) & ~0xfffffaaa;
		
		*PM_RSTS = asm_le32(PM_WDOG_MAGIC | rsts);
		*PM_WDOG = asm_le32(PM_WDOG_MAGIC | 10);
		*PM_RSTC = asm_le32(PM_WDOG_MAGIC | PM_RSTC_FULLRST);
		
		// Infinite loop => WatchDog will catch it and restart the system.
		
		while (1);
	}
	
	// FAIL if no PiStorm/Emu68 system.
	
	return (RETURN_FAIL);
}

/**********************************************************
 **
 ** End of file
 **
 **********************************************************/
