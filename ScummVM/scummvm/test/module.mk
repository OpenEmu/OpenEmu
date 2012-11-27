######################################################################
# Unit/regression tests, based on CxxTest.
# Use the 'test' target to run them.
# Edit TESTS and TESTLIBS to add more tests.
#
######################################################################

TESTS        := $(srcdir)/test/common/*.h $(srcdir)/test/audio/*.h
TEST_LIBS    := audio/libaudio.a common/libcommon.a

#
TEST_FLAGS   := --runner=StdioPrinter --no-std --no-eh --include=$(srcdir)/test/cxxtest_mingw.h
TEST_CFLAGS  := -I$(srcdir)/test/cxxtest
TEST_LDFLAGS := $(LIBS)
TEST_CXXFLAGS := $(filter-out -Wglobal-constructors,$(CXXFLAGS))

ifdef HAVE_GCC3
# In test/common/str.h, we test a zero length format string. This causes GCC
# to generate a warning which in turn poses a problem when building with -Werror.
# To work around this, we disable -Wformat here.
TEST_CFLAGS  +=  -Wno-format
endif

# Enable this to get an X11 GUI for the error reporter.
#TEST_FLAGS   += --gui=X11Gui
#TEST_LDFLAGS += -L/usr/X11R6/lib -lX11


test: test/runner
	./test/runner
test/runner: test/runner.cpp $(TEST_LIBS)
	$(QUIET_LINK)$(CXX) $(TEST_CXXFLAGS) $(CPPFLAGS) $(TEST_CFLAGS) -o $@ $+ $(TEST_LDFLAGS)
test/runner.cpp: $(TESTS)
	@mkdir -p test
	$(srcdir)/test/cxxtest/cxxtestgen.py $(TEST_FLAGS) -o $@ $+


clean: clean-test
clean-test:
	-$(RM) test/runner.cpp test/runner

.PHONY: test clean-test
