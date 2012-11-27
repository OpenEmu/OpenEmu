all: roll run

clean:
	rm -f *~ *.o roll test test.cpp

CXXTEST = ../..
CCFLAGS = -I. -I$(CXXTEST)

roll: roll.o Dice.o real_stdlib.o
	g++ -o $@ $^

run: test
	./test

test: test.o Dice.o mock_stdlib.o
	g++ -o $@ $^

.cpp.o:
	g++ -c -o $@ $(CCFLAGS) $<

test.cpp: TestDice.h
	$(CXXTEST)/cxxtestgen.pl -o $@ --error-printer $<
