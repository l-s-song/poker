CXX = g++
CXXFLAGS = -fmax-errors=1 -g -std=c++17 -Wall

.PHONY: debug prod

Card.o: Card.h

Deck.o: Deck.h Card.h

HandLogic.o: HandLogic.h Deck.h Card.h

HandSimulation.o: HandSimulation.h HandLogic.h Deck.h Card.h

PokerTerminal.o: HandSimulation.h HandLogic.h Deck.h Card.h

Server.o: HandSimulation.h.gch JSON.h APIFunctions.h Server.h HandSimulation.h HandLogic.h Deck.h Card.h

HandSimulation.h.gch: HandSimulation.h HandLogic.h Deck.h Card.h
	$(CXX) $(CXXFLAGS) -o HandSimulation.h.gch HandSimulation.h

HTTPServer.h.gch: HTTPServer.h
	$(CXX) $(CXXFLAGS) -o HTTPServer.h.gch HTTPServer.h

HTTPServer.o: HTTPServer.cpp HTTPServer.h.gch
	$(CXX) $(CXXFLAGS) -c HTTPServer.cpp

PokerTerminal: HandSimulation.h HandLogic.h Deck.h Card.h
	$(CXX) $(CXXFLAGS) -c PokerTerminal.cpp

DebugServer: HTTPServer.o Server.o HandSimulation.o HandLogic.o Deck.o Card.o
	$(CXX) $(CXXFLAGS) -pthread -o DebugServer HTTPServer.o Server.o HandSimulation.o HandLogic.o Deck.o Card.o -lboost_system

build-release-server:
	g++ -fmax-errors=1 -std=c++17 -O3 -pthread -o Server HTTPServer.cpp Server.cpp HandSimulation.cpp HandLogic.cpp Deck.cpp Card.cpp -lboost_system

terminal: PokerTerminal
	valgrind --log-file=valgrind.log ./PokerTerminal

debug: DebugServer
	valgrind --log-file=valgrind.log ./DebugServer

prod: build-release-server
	./Server
