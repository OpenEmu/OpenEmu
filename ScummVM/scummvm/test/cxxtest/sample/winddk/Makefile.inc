# -*- Makefile -*-

#
# Tell the DDK how to generate RunTests.cpp from RunTests.tpl and the tests
#

PERL=perl
PYTHON=python
CXXTESTGEN=$(PERL) $(CXXTESTDIR)/cxxtestgen.pl
#CXXTESTGEN=$(PYTHON) $(CXXTESTDIR)/cxxtestgen.py

TEST_SUITES=$(SUITESDIR)/*.h

RunTests.cpp: RunTests.tpl $(TEST_SUITES)
	$(CXXTESTGEN) -o $@ --template=RunTests.tpl $(TEST_SUITES)
