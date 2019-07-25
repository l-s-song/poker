#ifndef SERVER_H
#include "HandSimulation.h"

#include <shared_mutex>
#include <iostream>
#include <string>

enum game_type {
  nlhe,
  plo
};

enum game_format {
  ring,
  sitngo,
  tournament
};

struct Game {
  string id;
  string name;
  game_type type;
  game_format format;
  int table_size;
  int buy_in;
  int big_blind;
  int blind_timer;
  vector<string> player_ids;
  vector<string> tables;
  vector<string> leaving_players;

  string to_json();
  string to_json(int numtab);
};

struct Table {
  string id;
  string game_id;
  vector<string> player_ids;
  HandSimulation hand_sim;

  string to_json();
};

map<string, Game*> all_games;
map<string, Table*> all_tables;
map<string, shared_mutex*> game_mutexes;
map<string, shared_mutex*> table_mutexes;
map<string, string> session_id_to_player_id;
shared_mutex all_games_mutex;
shared_mutex all_tables_mutex;
shared_mutex session_id_to_player_id_mutex;
mutex queue_mutex;
typedef tuple<game_type, game_format, int, int> queue_settings;
map<queue_settings, vector<string>> queue;

#endif
#define SERVER_H
