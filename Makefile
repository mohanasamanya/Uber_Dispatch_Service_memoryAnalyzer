
# Uber Dispatch Memory Analyzer - Makefile

CC = gcc
CXX = g++

CFLAGS = -g -O0 -Wall -Wextra -pthread
TESTFLAGS = -DUNIT_TEST
COVERAGE = --coverage
GTEST_LIBS = -lgtest_main -lgtest -pthread

.PHONY: all run test valgrind coverage clean

# Default: compile the normal application
all: uber_dispatch

uber_dispatch: main.c uber.h
	$(CC) $(CFLAGS) main.c -o uber_dispatch

# Run the interactive Uber application
run: uber_dispatch
	./uber_dispatch

# Compile main.c for Google Test
main_test.o: main.c uber.h
	$(CC) $(CFLAGS) $(TESTFLAGS) -c main.c -o main_test.o

# Build the Google Test executable
uber_tests: uber_test.cpp main_test.o uber.h
	$(CXX) $(CFLAGS) uber_test.cpp main_test.o -o uber_tests $(GTEST_LIBS)

# Run Google Tests
test: uber_tests
	./uber_tests

# Check memory leaks with Valgrind
valgrind: uber_tests
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./uber_tests

# Run Google Tests with GCOV instrumentation
coverage:
	rm -f main_coverage.gcda main_coverage.gcno main.c.gcov
	$(CC) $(CFLAGS) $(COVERAGE) $(TESTFLAGS) -c main.c -o main_coverage.o
	$(CXX) $(CFLAGS) $(COVERAGE) uber_test.cpp main_coverage.o -o uber_tests_coverage $(GTEST_LIBS)
	./uber_tests_coverage
	gcov -b -c main.c -o main_coverage.o

# Remove generated files
clean:
	rm -f uber_dispatch uber_tests uber_tests_coverage
	rm -f *.o *.gcda *.gcno *.gcov
