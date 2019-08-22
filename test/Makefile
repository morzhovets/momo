COMPILER ?= g++ -std=c++11 -O2
CFLAGS = -v -Wall -Wextra -pedantic -Wno-unused-local-typedefs -msse2
TESTS = $(wildcard tests/*.cpp)

all: build/momo

build/momo:
	$(COMPILER) $(CFLAGS) -o build/momo main.cpp $(TESTS)

build/momo_gcc11:
	g++ -std=c++11 -O2 $(CFLAGS) -o build/momo_gcc11 main.cpp $(TESTS)

build/momo_clang11:
	clang++ -std=c++11 -O2 $(CFLAGS) -o build/momo_clang11 main.cpp $(TESTS)
