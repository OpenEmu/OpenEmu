%define ALIGN4 times ($$-$) & 3 nop
%define ALIGN32 times ($$-$) & 31 nop
%define ALIGN64 times ($$-$) & 63 nop

%define ALIGN_4 times ($$-$) & 3 resb 1
%define ALIGN_32 times ($$-$) & 31 resb 1
%define ALIGN_64 times ($$-$) & 63 resb 1

%macro DECL 1

	global _%1
	global %1

	_%1:
	%1:

%endmacro
;%define __GCC
%ifdef __GCC2
%macro DECLF 1-2

%if %0 > 1
	global %1%2
%endif
	global _%1
	global %1

%if %0 > 1
	%1%2:
%endif
	_%1:
	%1:

%endmacro
%else
%macro DECLF 1-2

%if %0 > 1
	global @%1@%2
%endif
	global _%1
	global %1

%if %0 > 1
	@%1@%2:
%endif
	_%1:
	%1:

%endmacro
%endif



BITS 32
