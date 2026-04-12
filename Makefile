.RECIPEPREFIX := >

CXX := g++
CXXFLAGS := -std=c++20 -Wall -pedantic -g -O2

# Options
DEBUG ?= 0
OPENMP ?= 1

ifeq ($(DEBUG), 1)
CXXFLAGS += -DEPI_DEBUG
endif

ifeq ($(OPENMP), 1)
CXXFLAGS += -fopenmp
endif

.PHONY: all clean

all: main.o

main.o: main.cpp epiworld.hpp
>$(CXX) $(CXXFLAGS) main.cpp -o main.o

clean:
>rm -f main.o

README.md: README.qmd main.o
>quarto render README.qmd