#ifndef __ASM_FUNCS_H__
#define __ASM_FUNCS_H__

/**********************************************************
 ** 
 ** INCLUDES
 ** 
 **********************************************************/

#include <exec/types.h>

/**********************************************************
 ** 
 ** DEFINES
 ** 
 **********************************************************/

#ifdef __GNUC__
#define ASM
#define REG(r,y) y __asm( # r )
#else 
#define ASM __asm __saveds
#define REG(r,y) register __ ## r y
#endif

/**********************************************************
 ** 
 ** PROTOTYPES FOR M68K MISC
 ** 
 **********************************************************/

VOID  ASM asm_nop(VOID);
ULONG ASM asm_le32(REG(d0, ULONG a));
ULONG ASM asm_div64(REG(d0, ULONG a), REG(d1, ULONG b), REG(d2, ULONG c));

#endif
