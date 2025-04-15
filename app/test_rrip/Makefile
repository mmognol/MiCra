CC := gcc
CXX := g++
CXXFLAGS := -std=c++2a -O3 -g -march=native -Wall -Wextra -Wpedantic -fconcepts -fopenmp
LDFLAGS := -fopenmp
DPULIBS := $(shell dpu-pkg-config --cflags --libs dpu)

TEST := test
SRC := cpu/main.cpp

.PHONY: all clean

all: $(TEST)

$(TEST): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(DPULIBS)
	cd ./dpu && make

clean:
	$(RM) $(TEST)
	cd ./dpu && make clean