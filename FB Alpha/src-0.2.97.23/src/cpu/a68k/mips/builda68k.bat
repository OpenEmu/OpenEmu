cl /nologo /Og /Oi /Ot /Oy /Ob2 /G6 /GF /Gy /GL /Gr /Zc:forScope /ML /Fp$(IntDir)\generated\ /DNDEBUG=1 /DINLINE="__inline static" /DWIN32 /DFASTCALL fba_make68k.c /link /NOLOGO /INCREMENTAL:NO /MACHINE:X86 /LIBPATH:"$(VCInstallDir)lib\"  /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:libcp.lib /NODEFAULTLIB:libcpd.lib /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcpmtd.lib /LTCG:STATUS /SUBSYSTEM:CONSOLE

fba_make68k a68k.s a68ktbl.inc
