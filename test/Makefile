COMPILER ?= g++ -std=c++11 -O2
CFLAGS = -v -Wall -Wextra -pedantic -Wsign-conversion -Wno-unused-local-typedefs -msse2
TESTS = $(wildcard tests/*.cpp)

all: build/momo

build/momo:
	$(COMPILER) $(CFLAGS) -o build/momo $(TESTS)

build/momo_gcc11:
	g++ -std=c++11 -O2 $(CFLAGS) -o build/momo_gcc11 $(TESTS)

build/momo_clang11:
	clang++ -std=c++11 -O2 $(CFLAGS) -o build/momo_clang11 $(TESTS)
