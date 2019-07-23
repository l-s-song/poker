CXX = g++
CXXFLAGS = -fmax-errors=1 -g -std=c++17 -Wall

Card.o: Card.h

Deck.o: Deck.h Card.h

HandLogic.o: HandLogic.h Deck.h Card.h

HandSimulation.o: HandSimulation.h HandLogic.h Deck.h Card.h

PokerTerminal.o: HandSimulation.h HandLogic.h Deck.h Card.h

Server.o: HandSimulation.h.gch Server.h HandSimulation.h HandLogic.h Deck.h Card.h

HandSimulation.h.gch: HandSimulation.h HandLogic.h Deck.h Card.h
	$(CXX) $(CXXFLAGS) -o HandSimulation.h.gch HandSimulation.h

HTTPServer.h.gch: HTTPServer.h
	$(CXX) $(CXXFLAGS) -o HTTPServer.h.gch HTTPServer.h

HTTPServer.o: HTTPServer.cpp HTTPServer.h.gch
	$(CXX) $(CXXFLAGS) -c HTTPServer.cpp

build-debug: HandSimulation.h HandLogic.h Deck.h Card.h
	$(CXX) $(CXXFLAGS) -c PokerTerminal.cpp

build-server: HTTPServer.o Server.o HandSimulation.o HandLogic.o Deck.o Card.o
	$(CXX) $(CXXFLAGS) -pthread -o Server HTTPServer.o Server.o HandSimulation.o HandLogic.o Deck.o Card.o -lboost_system

server: build-server
	valgrind --log-file=valgrind.log ./Server

debug: build-debug
	./poker

gdb: build-debug
	gdb poker

valgrind: build-debug
	valgrind ./poker
