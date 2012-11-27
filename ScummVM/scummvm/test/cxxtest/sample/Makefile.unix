#
# Makefile for UN*X-like systems
#

# Change this line if you want a different compiler
CXXC = c++ -Wall -W -Werror -I. -I..

# If you want to use python, specify USE_PYTHON=1 on the command line
ifdef USE_PYTHON
	TESTGEN = ../cxxtestgen.py
else
	TESTGEN = ../cxxtestgen.pl
endif

# For the X11 GUI
X11_FLAGS = -I/usr/X11R6/include -L/usr/X11R6/lib -lX11

# For the Qt GUI
#QTDIR = /usr/lib/qt
QTLIB = -lqt-mt
#QTLIB = -lqt
QT_FLAGS = -I$(QTDIR)/include -L$(QTDIR)/lib $(QTLIB) -O2

TARGETS = error_printer stdio_printer yes_no_runner file_printer aborter only
GUI_TARGETS = x11_runner qt_runner
TESTS = *.h
GUI_TESTS = gui/GreenYellowRed.h $(TESTS)

all: $(TARGETS)

clean:
	rm -f *~ *.o *.obj $(TARGETS) $(GUI_TARGETS)
	rm -f tests.cpp error_printer.cpp stdio_printer.cpp file_printer.cpp aborter.cpp only.cpp
	rm -f x11_runner.cpp qt_runner.cpp

distclean: clean
	rm -f Makefile

run: error_printer
	./error_printer

run_x11: x11_runner
	./x11_runner

run_qt: qt_runner
	./qt_runner

error_printer.cpp: $(TESTS)
	$(TESTGEN) -o $@ --error-printer $(TESTS)

stdio_printer.cpp: $(TESTS)
	$(TESTGEN) -o $@ --runner=StdioPrinter $(TESTS)

file_printer.cpp: file_printer.tpl $(TESTS)
	$(TESTGEN) -o $@ --template=file_printer.tpl $(TESTS)

aborter.cpp: aborter.tpl $(TESTS)
	$(TESTGEN) -o $@ --template=aborter.tpl $(TESTS)

only.cpp: only.tpl $(TESTS)
	$(TESTGEN) -o $@ --template=only.tpl $(TESTS)

tests.cpp: $(TESTS)
	$(TESTGEN) -o $@ $(TESTS)

x11_runner.cpp: $(GUI_TESTS)
	$(TESTGEN) -o $@ --gui=X11Gui $(GUI_TESTS)

qt_runner.cpp: $(GUI_TESTS)
	$(TESTGEN) -o $@ --gui=QtGui $(GUI_TESTS)

%: %.cpp
	$(CXXC) -o $@ $<

yes_no_runner: yes_no_runner.cpp tests.cpp
	$(CXXC) -o $@ $^

x11_runner: x11_runner.cpp
	$(CXXC) -o $@ $^ $(X11_FLAGS)

qt_runner: qt_runner.cpp
	$(CXXC) -o $@ $^ $(QT_FLAGS)

#
# Local Variables:
# compile-command: "make -fMakefile.unix"
# End:
#
