COMPILER=g++
OPTIONS=-std=c++20
LIBS=-lraylib

build:
	$(COMPILER) $(OPTIONS) -o bin/ego ego.cpp $(LIBS)

all: build
	./bin/ego

run:
	./bin/ego

clean:
	rm -rf bin/ego

