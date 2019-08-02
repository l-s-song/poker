#ifndef SERVER_H
#include "HandSimulation.h"

#include <shared_mutex>
#include <string>
#include <algorithm>
#include <ctime>

// In seconds
int default_blind_time = 600;

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
  vector<string> player_ids;
  vector<string> table_ids;
  vector<string> leaving_players;
  vector<string> waiting_players;

  // Tournament / SitnGo blind info
  int big_blind = -1;
  long long last_blind_level = -1;
  int time_between_blind_levels = -1;

  Game(string id, string name, game_settings& settings, vector<string> table_ids, vector<string> player_ids) {
    this->id = id;
    this->name = name;
    this->settings = settings;

    if(settings.format == sitngo || settings.format == tournament){
      big_blind = 2;
      time_between_blind_levels = default_blind_time;
      last_blind_level = (long long) std::time(nullptr);
    }

    this->player_ids = player_ids;
    this->table_ids = table_ids;
  };

  void update_blinds() {
    long long delta = ((long long) std::time(nullptr)) - last_blind_level;
    if (delta > time_between_blind_levels) {
      big_blind *= 2;
      last_blind_level = (long long) std::time(nullptr);
    }
  }

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
  bool four_colored_deck = false;
};

struct queue_entry {
  vector<game_type> types;
  game_format format;
  vector<int> table_sizes;
  int big_blind = -1;
  int buy_in = -1;
};

struct Player {
  string id;
  vector<queue_entry> queue_entries;
  vector<string> game_ids;
  vector<string> table_ids;
  player_settings settings;
  int chips = 0;
  int tokens = 0;
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
mutex rand_mutex;

#endif
#define SERVER_H
