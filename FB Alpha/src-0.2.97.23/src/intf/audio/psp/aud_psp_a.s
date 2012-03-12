
		 .set arch=allegrex
		 .set noreorder
		 .set nomacro
		 .set nobopt
		 .set noat

		 .globl pspSoundCopy44100
		 .globl pspSoundCopy22050
		 .globl pspSoundCopy11025

	.text


    .ent pspSoundCopy44100
pspSoundCopy44100:
	.frame $sp, 0, $ra

0:
	lw			$t0, ($a1)
	addiu		$a2, $a2, -1
	sw			$t0, ($a0)
	addiu		$a1, $a1, 4
	bne			$a2, $zero, 0b
	addiu		$a0, $a0, 4

	jr			$ra
	nop

	.end pspSoundCopy44100


    .ent pspSoundCopy22050
pspSoundCopy22050:
	.frame $sp, 0, $ra

0:
	lw			$t0, ($a1)
	addiu		$a2, $a2, -1
	sw			$t0, 0($a0)
	sw			$t0, 4($a0)
	addiu		$a1, $a1, 4
	bne			$a2, $zero, 0b
	addiu		$a0, $a0, 8

	jr			$ra
	nop

	.end pspSoundCopy22050


    .ent pspSoundCopy11025
pspSoundCopy11025:
	.frame $sp, 0, $ra

0:
	lw			$t0, ($a1)
	addiu		$a2, $a2, -1
	sw			$t0,  0($a0)
	sw			$t0,  4($a0)
	sw			$t0,  8($a0)
	sw			$t0, 12($a0)
	addiu		$a1, $a1, 4
	bne			$a2, $zero, 0b
	addiu		$a0, $a0, 16

	jr			$ra
	nop

	.end pspSoundCopy11025
