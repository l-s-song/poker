CXX = g++
CXXFLAGS = -fmax-errors=1 -g -std=c++11

build-debug:
	$(CXX) $(CXXFLAGS) Card.h Deck.h HandLogic.h HandSimulation.h Card.cpp Deck.cpp HandLogic.cpp HandSimulation.cpp PokerTerminal.cpp -o poker

debug: build-debug
	./poker

gdb: build-debug
	gdb poker

valgrind: build-debug
	valgrind ./poker
