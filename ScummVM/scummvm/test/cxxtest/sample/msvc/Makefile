# Where to look for the tests
TESTS            = ..\gui\*.h ..\*.h

# Where the CxxTest distribution is unpacked
CXXTESTDIR       = ..\..

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
CXXTESTGEN_FLAGS =        \
	--gui=Win32Gui        \
	--runner=ParenPrinter \
	--have-eh             \
	--abort-on-fail

# How to generate the test runner, `runner.cpp'
runner.cpp: $(TESTS)
	$(CXXTESTGEN) $(CXXTESTGEN_FLAGS) -o $@ $(TESTS)

# How to run the tests, which should be in DIR\runner.exe
run: $(DIR)\runner.exe
	$(DIR)\runner.exe
