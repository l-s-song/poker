#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ctime>
#include "Card.h"
#include "Deck.h"
#include "HandLogic.h"
#include "HandSimulation.h"

using namespace std;



int main() {
  srand(time(NULL));
  int numplayers;
  cout << "How many players?" << endl;
  cin >> numplayers;
  cout << "Enter player first names." << endl;
  HandSimulation game(1, 2, 100);
  game.players = vector<Player>(numplayers);
  for(int i = 0; i < numplayers; i++){
    cin >> game.players[i].name;
    game.stacks[i] = game.bigBlind*100;
  }
  cin.ignore();
  game.PlayGame();
  return 0;
}
