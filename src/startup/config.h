/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework for ARM7 processor
 *
 *****************************************************************************/
#ifndef _config_h_
#define _config_h_


/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/

#define FOSC    14745600 //12000000             /* External clock input frequency (must be between 10 MHz and 25 MHz) */

#define USE_PLL 1                     /* 0 = do not use on-chip PLL,
                                         1 = use on-chip PLL) */
#define PLL_MUL 4                     /* PLL multiplication factor (1 to 32) */
#define PLL_DIV 2                     /* PLL division factor (1, 2, 4, or 8) */
#define PBSD    4                     /* Peripheral bus speed divider (1, 2, or 4) */

#define CRYSTAL_FREQUENCY FOSC //14745600 
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

/* initialize the MAM (Memory Accelerator Module) */
#if USE_PLL == 1
#define CORE_FREQ (FOSC * PLL_MUL)
#else
#define CORE_FREQ (FOSC)
#endif

#if CORE_FREQ < 20000000
#define MAM_TIMING   1                /* number of CCLK to read from the FLASH */
#elif CORE_FREQ < 40000000
#define MAM_TIMING   2                /* number of CCLK to read from the FLASH */
#else
#define MAM_TIMING   3                /* number of CCLK to read from the FLASH */
#endif

#define MAM_SETTING  2                /* 0=disabled,
                                         1=partly enabled (enabled for code prefetch, but not for data),
                                         2=fully enabled */

#define IRQ_HANDLER  1                /* 0 = Jump to common IRQ handler
                                         1 = Load vector directly from VIC, i.e., LDR PC,[PC,#-0xFF0] */

/* initialize the exception vector mapping */
#ifndef RAM_EXEC
#define MAM_MAP      1                  /* 1 = exception vectors are in FLASH at 0x0000 0000,
                                           2 = exception vectors are in SRAM at 0x4000 0000   */
#else
#define MAM_MAP 2                       /* When executing from RAM, MAM_MAP should always be 2 */
#endif

/* setup stack sizes */
#define stackSize_SYS    600
#define stackSize_SVC     64
#define stackSize_UND     64
#define stackSize_ABT     64
#define stackSize_IRQ    600
#define stackSize_FIQ     64


/* define SRAM size */
#ifdef LPC2101
#define SRAM_SIZE  (2 * 1024)   /* LPC2101 */
#elif defined (LPC2102)
#define SRAM_SIZE  (4 * 1024)   /* LPC2102 */
#elif defined (LPC2103)
#define SRAM_SIZE  (8 * 1024)   /* LPC2103 */
#elif defined (LPC2104)
#define SRAM_SIZE  (16 * 1024)   /* LPC2104 */
#elif defined (LPC2105)
#define SRAM_SIZE  (32 * 1024)   /* LPC2105 */
#elif defined (LPC2106)
#define SRAM_SIZE  (64 * 1024)   /* LPC2106 */

#elif defined (LPC2114)
#define SRAM_SIZE  (16 * 1024)   /* LPC2114 */
#elif defined (LPC2124)
#define SRAM_SIZE  (16 * 1024)   /* LPC2124 */
#elif defined (LPC2119)
#define SRAM_SIZE  (16 * 1024)   /* LPC2119 */
#elif defined (LPC2129)
#define SRAM_SIZE  (16 * 1024)   /* LPC2129 */

#elif defined (LPC2131)
#define SRAM_SIZE  (8 * 1024)   /* LPC2131 */
#elif defined (LPC2132)
#define SRAM_SIZE  (16 * 1024)   /* LPC2132 */
#elif defined (LPC2134)
#define SRAM_SIZE  (16 * 1024)   /* LPC2134 */
#elif defined (LPC2136)
#define SRAM_SIZE  (32 * 1024)   /* LPC2136 */
#elif defined (LPC2138)
#define SRAM_SIZE  (32 * 1024)   /* LPC2138 */

#elif defined (LPC2141)
#define SRAM_SIZE  (8 * 1024)   /* LPC2141 */
#elif defined (LPC2142)
#define SRAM_SIZE  (16 * 1024)   /* LPC2142 */
#elif defined (LPC2144)
#define SRAM_SIZE  (16 * 1024)   /* LPC2144 */
#elif defined (LPC2146)
#define SRAM_SIZE  (32 * 1024)   /* LPC2146 */
#elif defined (LPC2148)
#define SRAM_SIZE  (32 * 1024)   /* LPC2148 */

#elif defined (LPC2194)
#define SRAM_SIZE  (16 * 1024)   /* LPC2194 */

#elif defined (LPC2210)
#define SRAM_SIZE  (16 * 1024)   /* LPC2210 */
#elif defined (LPC2220)
#define SRAM_SIZE  (64 * 1024)   /* LPC2220 */
#elif defined (LPC2212)
#define SRAM_SIZE  (16 * 1024)   /* LPC2212 */
#elif defined (LPC2214)
#define SRAM_SIZE  (16 * 1024)   /* LPC2214 */
#elif defined (LPC2290)
#define SRAM_SIZE  (16 * 1024)   /* LPC2290 */
#elif defined (LPC2292)
#define SRAM_SIZE  (16 * 1024)   /* LPC2292 */
#elif defined (LPC2294)
#define SRAM_SIZE  (16 * 1024)   /* LPC2294 */

#else
#error CPU_VARIANT not defined in the makefile, or illegal value 
#endif

#define SRAM_SADDR   0x40000000               /* SRAM starting address */
#define SRAM_TOP     (SRAM_SADDR+SRAM_SIZE)   /* SRAM end address + 1 */
#define SRAM_EADDR   (SRAM_SADDR+SRAM_SIZE-1) /* SRAM end address */

#define STK_SIZE     (stackSize_SYS+stackSize_SVC+stackSize_UND+stackSize_ABT+stackSize_IRQ+stackSize_FIQ)
#define STK_SADDR    (SRAM_EADDR+1-STK_SIZE)  /* Stack start address */

#endif  /* _config_h_ */
