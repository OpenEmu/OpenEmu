#ifndef M68KCONF__HEADER
#define M68KCONF__HEADER

/* ======================================================================== */
/* ======================== SUB 68K CONFIGURATION ========================= */
/* ======================================================================== */

/* Configuration switches.
 * Use OPT_SPECIFY_HANDLER for configuration options that allow callbacks.
 * OPT_SPECIFY_HANDLER causes the core to link directly to the function
 * or macro you specify, rather than using callback functions whose pointer
 * must be passed in using m68k_set_xxx_callback().
 */
#define OPT_OFF             0
#define OPT_ON              1
#define OPT_SPECIFY_HANDLER 2

/* If ON, the CPU will call m68k_write_32_pd() when it executes move.l with a
 * predecrement destination EA mode instead of m68k_write_32().
 * To simulate real 68k behavior, m68k_write_32_pd() must first write the high
 * word to [address+2], and then write the low word to [address].
 */
#define M68K_SIMULATE_PD_WRITES     OPT_OFF

/* If ON, CPU will call the interrupt acknowledge callback when it services an
 * interrupt.
 * If off, all interrupts will be autovectored and all interrupt requests will
 * auto-clear when the interrupt is serviced.
 */
#define M68K_EMULATE_INT_ACK        OPT_SPECIFY_HANDLER
#define M68K_INT_ACK_CALLBACK(A)    scd_68k_irq_ack(A)

/* If ON, CPU will call the output reset callback when it encounters a reset
 * instruction.
 */
#define M68K_EMULATE_RESET          OPT_OFF
#define M68K_RESET_CALLBACK()       your_reset_handler_function()

/* If ON, CPU will call the callback when it encounters a tas
 * instruction.
 */
#define M68K_TAS_HAS_CALLBACK       OPT_SPECIFY_HANDLER
#define M68K_TAS_CALLBACK()         1

/* If ON, CPU will call the set fc callback on every memory access to
 * differentiate between user/supervisor, program/data access like a real
 * 68000 would.  This should be enabled and the callback should be set if you
 * want to properly emulate the m68010 or higher. (moves uses function codes
 * to read/write data from different address spaces)
 */
#define M68K_EMULATE_FC             OPT_OFF
#define M68K_SET_FC_CALLBACK(A)     your_set_fc_handler_function(A)

/* If ON, the CPU will monitor the trace flags and take trace exceptions
 */
#define M68K_EMULATE_TRACE          OPT_OFF

/* If ON, the CPU will emulate the 4-byte prefetch queue of a real 68000 */
#define M68K_EMULATE_PREFETCH       OPT_OFF

/* If ON, the CPU will generate address error exceptions if it tries to
 * access a word or longword at an odd address.
 * NOTE: This is only emulated properly for 68000 mode.
 */
#define M68K_EMULATE_ADDRESS_ERROR  OPT_OFF

/* If ON and previous option is also ON, address error exceptions will
   also be checked when fetching instructions. Disabling this can help
   speeding up emulation while still emulating address error exceptions
   on other memory access if needed.
 * NOTE: This is only emulated properly for 68000 mode.
 */
#define M68K_CHECK_PC_ADDRESS_ERROR OPT_OFF


/* ----------------------------- COMPATIBILITY ---------------------------- */

/* The following options set optimizations that violate the current ANSI
 * standard, but will be compliant under the forthcoming C9X standard.
 */


/* If ON, the enulation core will use 64-bit integers to speed up some
 * operations.
*/
#define M68K_USE_64_BIT  OPT_OFF


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KCONF__HEADER */
