#include "HandSimulation.h"
#include "Server.h"
#include "JSON.h"

#include <shared_mutex>
#include <iostream>

struct Game {
  string id;
  game_type type;
  game_format format;
  int table_size;
  int buy_in;
  int big_blind;
  int blind_timer;
  vector<string> tables;

  string to_json() {
    string s = "{\n";
    s += format_json("id", id, 1);
    s += format_json("type", to_string(type), 1);
    s += format_json("format", to_string(format), 1);
    s += format_json("table_size", table_size, 1);
    s += format_json("buy_in", buy_in, 1);
    s += format_json("big_blind", big_blind, 1);
    s += format_json("blind_timer", blind_timer, 1);
    s += format_json("tables", tables, 1, false);
    s += "}\n";
    return s;
  };
};


struct Table {
  string id;
  string game_id;
  vector<string> player_ids;
  HandSimulation hand_sim;

  string to_json() {
    string s = "{\n";
    s += format_json("id", id, 1);
    s += format_json("game_id", game_id, 1);
    s += format_json("player_ids", player_ids, 1);
    s += format_json("board", hand_sim.getBoard(), 1);
    s += format_json("pots", hand_sim.getPots(), 1);
    s += format_json("bets", hand_sim.getBets(), 1);
    s += format_json("stacks", hand_sim.getStacks(), 1);
    s += format_json("button_location", hand_sim.getButtonLocation(), 1);
    s += format_json("current_turn", hand_sim.getCurrentTurn(), 1);
    s += format_json("active_players", hand_sim.getActivePlayers(), 1);
    s += format_json("current_bet", hand_sim.getCurrentBet(), 1);
    s += format_json("min_raise", hand_sim.getMinRaise(), 1, false);
    s += "}\n";
    return s;
  };
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

string get_player_id(string session_id) {
  session_id_to_player_id_mutex.lock_shared();
  string player_id = "";
  if (session_id_to_player_id.count(session_id)) {
    player_id = session_id_to_player_id[session_id];
  }
  session_id_to_player_id_mutex.unlock_shared();
  return player_id;
}

void set_player_id(string session_id, string player_id) {
  session_id_to_player_id_mutex.lock();
  session_id_to_player_id[session_id] = player_id;
  session_id_to_player_id_mutex.unlock();
}

mutex rand_mutex;

int gen_rand() {
  int ret;
  rand_mutex.lock();
  ret = rand();
  rand_mutex.unlock();
  return ret;
}

void init_server() {
  srand(time(NULL));
  HandSimulation hs(2, 0, vector<int>(6, 200));
  hs.initBettingRound();
  all_tables["5"] = new Table {
    "5",
    "1234",
    {"a", "b", "c", "d", "e", "f"},
    hs
  };
  table_mutexes["5"] = new shared_mutex;
  all_games["1234"] = new Game {
    "1234",
    nlhe,
    sitngo,
    6,
    200,
    2,
    15,
    {"5"}
  };
  game_mutexes["1234"] = new shared_mutex;
  queue_settings qs = make_tuple(nlhe, ring, 6, 2);
  queue[qs];
}


string player_act(string session_id, string table_id, string action, int bet_size) {
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return error_json("Player not logged in");
  }
  string ret;
  all_tables_mutex.lock_shared();
  if (all_tables.count(table_id)) {
    table_mutexes[table_id]->lock();
    vector<string>& player_ids = all_tables[table_id]->player_ids;
    HandSimulation& hs = all_tables[table_id]->hand_sim;
    if (player_ids[hs.getCurrentTurn()] != player_id) {
      ret = error_json("It is not your turn");
    } else {
      if (action == "raise") {
        if (bet_size <= hs.getCurrentBet()) {
          ret = error_json("Raise must be larger than the curent bet");
        } else if (!hs.isValidBet(bet_size)) {
          ret = error_json("Raise size is not valid")
        } else {
          hs.Bet(bet_size);
        }
      } else if (action == "bet") {
        if (hs.getCurrentBet() != 0) {
          ret = error_json("Bet cannot be made as a bet already exists");
        } else if (!hs.isValidBet(bet_size)) {
          ret = error_json("Bet size is not valid")
        } else {
          hs.Bet(bet_size);
        }
      } else if (action == "call") {
        if (hs.canCheck()) {
          ret = error_json("Cannot call if check is available");
        } else {
          hs.Call();
        }
      } else if (action == "check") {
        if (!hs.canCheck()) {
          ret = error_json("Cannot check, the current bet is " + to_string(hs.getCurrentBet()));
        } else {
          hs.Check();
        }
      } else if (action == "fold") {
        hs.Fold();
      } else {
        ret = error_json(action + " is not a valid action");
      }
      while(hs.isBettingRoundOver()) {
        hs.endBettingRound();
        if (hs.isHandOver()) {
          break;
        }
        hs.initBettingRound();
      }
      if (hs.isHandOver()) {
        // Unimplemented
      }
    }
    table_mutexes[table_id]->unlock();
  }
  all_tables_mutex.unlock_shared();
  return ret;
}

string add_to_queue(string session_id, string type, string format, int table_size, int buy_in_or_big_blind) {
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return error_json("Player not logged in");
  }
  string invalid_settings = error_json("A queue with those settings does not exist");
  if (!is_game_type(type) || !is_game_format(format)) {
    return invalid_settings;
  }

  string ret = "{}";
  queue_settings input = {to_game_type(type), to_game_format(format), table_size, buy_in_or_big_blind};
  queue_mutex.lock();
  if (queue.count(input)) {
    vector<string>& player_ids = queue[input];
    player_ids.push_back(player_id);
    if(player_ids.size() == table_size){
      //make new game
      string table_id = to_string(gen_rand());
      string game_id = to_string(gen_rand());
      HandSimulation hs(buy_in_or_big_blind, 0, vector<int>(table_size, 100*buy_in_or_big_blind));
      hs.initBettingRound();
      Table* table = new Table{table_id, game_id, player_ids, hs};
      Game* game = new Game {
        game_id, get<0>(input), get<1>(input), get<2>(input), -1, get<3>(input), -1, {table_id}
      };

      all_games_mutex.lock();
      all_games[game_id] = game;
      game_mutexes[game_id] = new shared_mutex;
      all_games_mutex.unlock();

      all_tables_mutex.lock();
      all_tables[table_id] = table;
      table_mutexes[table_id] = new shared_mutex;
      all_tables_mutex.unlock();

      ret = "{\n"
          + format_json("game_id", game_id, 1, false)
          + "}\n";

      player_ids.clear();
    }
  } else {
    ret = invalid_settings;
  }
  queue_mutex.unlock();
  return ret;
}

string get_game_from_id(string game_id) {
  string response;
  all_games_mutex.lock_shared();
  if(all_games.count(game_id) > 0){
    game_mutexes[game_id]->lock_shared();
    Game& game = *all_games[game_id];
    response = game.to_json();
    game_mutexes[game_id]->unlock_shared();
  } else {
    response = error_json("Game ID Not Found");
  }
  all_games_mutex.unlock_shared();

  return response;
};

string get_table_from_id(string table_id) {
  string response;
  all_tables_mutex.lock_shared();
  if(all_tables.count(table_id)){
    table_mutexes[table_id]->lock_shared();
    Table& thetable = *all_tables[table_id];
    response = thetable.to_json();
    table_mutexes[table_id]->unlock_shared();
  } else {
    response = error_json("Table ID Not Found");
  }
  all_tables_mutex.unlock_shared();

  return response;
};
