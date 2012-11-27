@rem = '--*-Perl-*--
@echo off
if "%OS%" == "Windows_NT" goto WinNT
perl -x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
:WinNT
perl -x -S %0 %*
if NOT "%COMSPEC%" == "%SystemRoot%\system32\cmd.exe" goto endofperl
if %errorlevel% == 9009 echo You do not have Perl in your PATH.
if errorlevel 1 goto script_failed_so_exit_with_non_zero_val 2>nul
goto endofperl
@rem ';
#!/usr/bin/perl -w
#line 15
use strict;
use English;
use Getopt::Long;

$OUTPUT_AUTOFLUSH = 1;

sub usage() {
  print STDERR "Usage: $0 <OPTIONS>\n\n";
  print STDERR "Fix Makefile and CxxTest_2_Build.dsp for your setup.\n\n";
  print STDERR "  --cxxtest=DIR   Assume CxxTest is installed in DIR (default: '..\\..')\n";
  print STDERR "  --tests=SPEC    Use SPEC for the test files (default: '../gui/*.h ../*.h')\n\n";
  print STDERR "You must specify at least one option.\n";
  exit -1;
}

my ($cxxtest, $tests);
my ($Makefile, $CxxTest_2_Build);

sub main {
  parseCommandline();
  fixFiles();
}

sub parseCommandline() {
  GetOptions( 'cxxtest=s' => \$cxxtest,
	      'tests=s'   => \$tests,
	    ) or usage();

  usage() unless (defined($cxxtest) || defined($tests));
  $cxxtest = '..\\..' unless defined($cxxtest);
  $tests = '../gui/*.h ../*.h' unless defined($tests);
}

sub fixFiles() {
  fixFile( $Makefile, 'Makefile' );
  fixFile( $CxxTest_2_Build, 'CxxTest_2_Build.dsp' );
}

sub fixFile($$) {
  my ($data, $output) = @_;

  print "$output...";

  $data =~ s/<TESTS>/$tests/g;
  $data =~ s/<CXXTEST>/$cxxtest/g;

  open OUTPUT, ">$output" or die "Cannot create output file \"$output\"\n";
  print OUTPUT $data;
  close OUTPUT;

  print "OK\n";
}

$Makefile =
'# Where to look for the tests
TESTS            = <TESTS>

# Where the CxxTest distribution is unpacked
CXXTESTDIR       = <CXXTEST>

# Check CXXTESTDIR
!if !exist($(CXXTESTDIR)\cxxtestgen.pl)
!error Please fix CXXTESTDIR
!endif

# cxxtestgen needs Perl or Python
!if defined(PERL)
CXXTESTGEN       = $(PERL) $(CXXTESTDIR)/cxxtestgen.pl
!elseif defined(PYTHON)
CXXTESTGEN       = $(PYTHON) $(CXXTESTDIR)/cxxtestgen.py
!else
!error You must define PERL or PYTHON
!endif

# The arguments to pass to cxxtestgen
#  - ParenPrinter is the way MSVC likes its compilation errors
#  - --have-eh/--abort-on-fail are nice when you have them
CXXTESTGEN_FLAGS = --gui=Win32Gui --runner=ParenPrinter --have-eh --abort-on-fail

# How to generate the test runner, "runner.cpp"
runner.cpp: $(TESTS)
        $(CXXTESTGEN) $(CXXTESTGEN_FLAGS) -o $@ $(TESTS)

# Command-line arguments to the runner
RUNNER_FLAGS = -title "CxxTest Runner"

# How to run the tests, which should be in DIR\runner.exe
run: $(DIR)\runner.exe
        $(DIR)\runner.exe $(RUNNER_FLAGS)
';

$CxxTest_2_Build = 
'# Microsoft Developer Studio Project File - Name="CxxTest_2_Build" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=CxxTest_2_Build - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CxxTest_2_Build.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CxxTest_2_Build.mak" CFG="CxxTest_2_Build - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CxxTest_2_Build - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "CxxTest_2_Build - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CxxTest_2_Build - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "<CXXTEST>" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x40d /d "NDEBUG"
# ADD RSC /l 0x40d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Release/runner.exe"

!ELSEIF  "$(CFG)" == "CxxTest_2_Build - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "<CXXTEST>" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40d /d "_DEBUG"
# ADD RSC /l 0x40d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/runner.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CxxTest_2_Build - Win32 Release"
# Name "CxxTest_2_Build - Win32 Debug"
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\runner.cpp
# End Source File
# End Target
# End Project
';

main();

__END__
:endofperl

rem
rem Local Variables:
rem compile-command: "perl FixFiles.bat"
rem End:
rem
