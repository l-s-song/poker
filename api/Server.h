#ifndef SERVER_H
#include "HandSimulation.h"

#include <shared_mutex>
#include <string>

int default_blind_timer = 10;

enum game_type {
  nlhe,
  plo
};

enum game_format {
  ring,
  sitngo,
  tournament
};

// type, format, table_size, buy_in_or_big_blind
struct game_settings {
  game_type type;
  game_format format;
  int table_size;
  int buy_in = -1;
  int big_blind = -1;
};

bool operator< (const game_settings& a, const game_settings& b) {
  return make_tuple(a.type, a.format, a.table_size, a.buy_in, a.big_blind) <
         make_tuple(b.type, b.format, b.table_size, b.buy_in, b.big_blind);
}

struct Game {
  string id;
  string name;
  game_settings settings;
  // TODO: Tournaments
  //int big_blind = -1;
  //int blind_timer = -1;
  vector<string> player_ids;
  vector<string> table_ids;
  vector<string> leaving_players;
  vector<string> waiting_players;

  Game(string id, string name, game_settings& settings, vector<string> table_ids, vector<string> player_ids) {
    this->id = id;
    this->name = name;
    this->settings = settings;
    //tournament functionality
    /*
    if(settings.format == sitngo || settings.format == tournament){
      big_blind = 2;
      blind_timer = default_blind_timer;
    }
    */
    this->player_ids = player_ids;
    this->table_ids = table_ids;
  };

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

struct player_settings {

};

struct queue_entry {
  vector<game_type> types;
  game_format format;
  vector<int> table_sizes;
};

struct Player {
  vector<queue_entry> queue_entries;
  player_settings settings;
  int chips;
  int tokens;
  vector<string> game_ids;
  vector<string> table_ids;
};

map<string, Player*> all_players;
map<string, Game*> all_games;
map<string, Table*> all_tables;
map<string, shared_mutex*> player_mutexes;
map<string, shared_mutex*> game_mutexes;
map<string, shared_mutex*> table_mutexes;
map<string, string> session_id_to_player_id;
shared_mutex all_players_mutex;
shared_mutex all_games_mutex;
shared_mutex all_tables_mutex;
shared_mutex session_id_to_player_id_mutex;
shared_mutex queue_mutex;
map<game_settings, vector<string>> queue;

#endif
#define SERVER_H
