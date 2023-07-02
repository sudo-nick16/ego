build:
	g++ -std=c++20 -o bin/ego ego.cpp

run: build
	./bin/ego

