TESTS = $(wildcard tests/*.cpp)
CFLAGS = -Wall -Wno-unused-local-typedefs

all: clean build/momo_gcc11 build/momo_clang11

clean:
	rm -rf build
	mkdir build

build/momo_gcc11:
	g++ -std=c++11 $(CFLAGS) -o build/momo_gcc11 main.cpp $(TESTS)

build/momo_gcc17:
	g++ -std=c++17 $(CFLAGS) -o build/momo_gcc17 main.cpp $(TESTS)

build/momo_clang11:
	clang++ -std=c++11 $(CFLAGS) -o build/momo_clang11 main.cpp $(TESTS)

build/momo_clang17:
	clang++ -std=c++17 $(CFLAGS) -o build/momo_clang17 main.cpp $(TESTS)

