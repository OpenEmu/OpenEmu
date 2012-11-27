@---------------------------------------------------------------------------------
	.align	4
	.arm
	.global _io_dldi
	.global _dldi_driver_name
@---------------------------------------------------------------------------------
.equ FEATURE_MEDIUM_CANREAD,		0x00000001
.equ FEATURE_MEDIUM_CANWRITE,		0x00000002
.equ FEATURE_SLOT_GBA,				0x00000010
.equ FEATURE_SLOT_NDS,				0x00000020


@---------------------------------------------------------------------------------
@ Driver patch file standard header -- 16 bytes
	.word	0xBF8DA5ED		@ Magic number to identify this region
	.asciz	" Chishm"		@ Identifying Magic string (8 bytes with null terminator)
	.byte	0x01			@ Version number
	.byte	0x0F	@32KiB	@ Log [base-2] of the size of this driver in bytes.
	.byte	0x00			@ Sections to fix
	.byte 	0x0F	@32KiB	@ Log [base-2] of the allocated space in bytes.

@---------------------------------------------------------------------------------
@ Text identifier - can be anything up to 47 chars + terminating null -- 16 bytes
	.align	4
_dldi_driver_name:
	.asciz "Default (No interface)"

@---------------------------------------------------------------------------------
@ Offsets to important sections within the data	-- 32 bytes
	.align	6
	.word   0x00000000		@ data start
	.word   0x00000000		@ data end
	.word	0x00000000		@ Interworking glue start	-- Needs address fixing
	.word	0x00000000		@ Interworking glue end
	.word   0x00000000		@ GOT start					-- Needs address fixing
	.word   0x00000000		@ GOT end
	.word   0x00000000		@ bss start					-- Needs setting to zero
	.word   0x00000000		@ bss end

@---------------------------------------------------------------------------------
@ IO_INTERFACE data -- 32 bytes
_io_dldi:
	.ascii	"DLDI"					@ ioType
	.word	0x00000000				@ Features
	.word	_DLDI_startup			@
	.word	_DLDI_isInserted		@
	.word	_DLDI_readSectors		@   Function pointers to standard device driver functions
	.word	_DLDI_writeSectors		@
	.word	_DLDI_clearStatus		@
	.word	_DLDI_shutdown			@

@---------------------------------------------------------------------------------

_DLDI_startup:
_DLDI_isInserted:
_DLDI_readSectors:
_DLDI_writeSectors:
_DLDI_clearStatus:
_DLDI_shutdown:
	mov		r0, #0x00				@ Return false for every function
	bx		lr



@---------------------------------------------------------------------------------
	.align
	.pool

.space 32632						@ Fill to 32KiB

	.end
@---------------------------------------------------------------------------------
