CXX = g++
CXXFLAGS = -fmax-errors=1 -g -std=c++11

Card.o: Card.h

Deck.o: Deck.h Card.h

HandLogic.o: HandLogic.h Deck.h Card.h

HandSimulation.o: HandSimulation.h HandLogic.h Deck.h Card.h

PokerTerminal.o: HandSimulation.h HandLogic.h Deck.h Card.h

Server.h.gch: Server.h HandSimulation.h HandLogic.h Deck.h Card.h
	$(CXX) $(CXXFLAGS) -o Server.h.gch Server.h

Server.o: Server.cpp Server.h.gch
	$(CXX) $(CXXFLAGS) -c Server.cpp

build-debug: HandSimulation.h HandLogic.h Deck.h Card.h
	$(CXX) $(CXXFLAGS) -c PokerTerminal.cpp

server: Server.o HandSimulation.o HandLogic.o Deck.o Card.o
	$(CXX) $(CXXFLAGS) -pthread -o Server Server.o HandSimulation.o HandLogic.o Deck.o Card.o -lboost_system


debug: build-debug
	./poker

gdb: build-debug
	gdb poker

valgrind: build-debug
	valgrind ./poker
