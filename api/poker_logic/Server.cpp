#include "Server.h"
#include "JSON.h"
#include "APIFunctions.h"

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

int generate_int() {
  int ret;
  rand_mutex.lock();
  ret = rand();
  rand_mutex.unlock();
  return ret;
}

string generate_id() {
  return to_string(generate_int());
}

void init_server() {
  srand(time(NULL));
  queue_settings qs = make_tuple(nlhe, ring, 6, 2);
  queue[qs];
}

string get_games() {
  string ret = "[\n";
  all_games_mutex.lock_shared();
  bool first = true;
  for(auto g : all_games) {
    if (first) {
      first = false;
    } else {
      ret += ",\n";
    }
    ret += g.second->to_json(1);
  }
  if (!first) {
    ret += "\n";
  }
  all_games_mutex.unlock_shared();
  ret += "]\n";
  return ret;
}

string player_act(string session_id, string table_id, string action, int bet_size) {
  //poker action posts
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return error_json("Player not logged in");
  }
  string ret = "{}\n";
  all_tables_mutex.lock_shared();
  if (all_tables.count(table_id)) {
    table_mutexes[table_id]->lock();
    vector<string>& player_ids = all_tables[table_id]->player_ids;
    HandSimulation& hs = all_tables[table_id]->hand_sim;
    if (hs.isHandOver()) {
      ret = error_json("Hand is already over");
    } else if (player_ids[hs.getCurrentTurn()] != player_id) {
      ret = error_json("It is not your turn");
    } else {
      if (action == "raise") {
        if (bet_size <= hs.getCurrentBet()) {
          ret = error_json("Raise must be larger than the curent bet");
        } else if (!hs.isValidBet(bet_size)) {
          ret = error_json("Raise size is not valid");
        } else {
          hs.Bet(bet_size);
        }
      } else if (action == "bet") {
        if (hs.getCurrentBet() != 0) {
          ret = error_json("Bet cannot be made as a bet already exists");
        } else if (!hs.isValidBet(bet_size)) {
          ret = error_json("Bet size is not valid");
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

// queue_mutex must be locked in order to call this
void distribute() {
  for(auto specific_queue : queue) {
    queue_settings settings = specific_queue->first;
    vector<string>& player_ids = specific_queue->second;
    while(player_ids.size() > table_size/2){
      //make new game
      vector<string> next_player_ids;
      
      string table_id = generate_id();
      string game_id = generate_id();
      HandSimulation hs(buy_in_or_big_blind, 0, vector<int>(table_size, 100*buy_in_or_big_blind));
      hs.initBettingRound();
      Table* table = new Table{table_id, game_id, player_ids, hs};
      Game* game = new Game {
        game_id, "NYC", get<0>(settings), get<1>(settings), get<2>(settings), -1, get<3>(settings), -1, player_ids, {table_id}, {}
      };

      all_tables_mutex.lock();
      all_tables[table_id] = table;
      table_mutexes[table_id] = new shared_mutex;
      all_tables_mutex.unlock();

      all_games_mutex.lock();
      all_games[game_id] = game;
      game_mutexes[game_id] = new shared_mutex;
      all_games_mutex.unlock();

      ret = "{\n"
          + format_json("game_id", game_id, 1, false)
          + "}\n";

      player_ids.clear();
    }
  }
}

string add_to_queue(string session_id, string type, string format, int table_size, int buy_in_or_big_blind) {
  //player joining game
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return error_json("Player not logged in");
  }
  string invalid_settings = error_json("A queue with those settings does not exist");
  if (!is_game_type(type) || !is_game_format(format)) {
    return invalid_settings;
  }

  string ret = "{}";
  queue_settings settings = {to_game_type(type), to_game_format(format), table_size, buy_in_or_big_blind};
  queue_mutex.lock();
  if (queue.count(settings)) {
    vector<string>& player_ids = queue[settings];
    player_ids.push_back(player_id);
    distribute();
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
    response = game.to_json() + "\n";
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
    response = thetable.to_json() + "\n";
    table_mutexes[table_id]->unlock_shared();
  } else {
    response = error_json("Table ID Not Found");
  }
  all_tables_mutex.unlock_shared();

  return response;
};
