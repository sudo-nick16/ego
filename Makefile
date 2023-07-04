build:
	g++ -std=c++20 -o bin/ego ego.cpp -lraylib 

all: build
	./bin/ego

run:
	./bin/ego

clean:
	rm -rf bin/ego

