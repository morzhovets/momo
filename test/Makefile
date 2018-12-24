TESTS = $(wildcard tests/*.cpp)
CFLAGS = -Wall -Wno-unused-local-typedefs -msse2

all: clean build/momo_gcc11 build/momo_clang11

clean:
	rm -rf build
	mkdir build

build/momo_gcc11:
	g++ -std=c++11 $(CFLAGS) -o build/momo_gcc11 $(TESTS)

build/momo_gcc17:
	g++ -std=c++17 $(CFLAGS) -o build/momo_gcc17 $(TESTS)

build/momo_clang11:
	clang++ -std=c++11 $(CFLAGS) -o build/momo_clang11 $(TESTS)

build/momo_clang17:
	clang++ -std=c++17 $(CFLAGS) -o build/momo_clang17 $(TESTS)

