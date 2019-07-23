#include "HandSimulation.h"
#include "Server.h"

#include <shared_mutex>
#include <iostream>

enum game_type {
  nlhe,
  plo
};

string to_string(game_type type){
  if(type == nlhe){
    return "nlhe";
  } else {
    return "plo";
  }
}

game_type to_game_type(string s){
  if(s == "nlhe"){
    return nlhe;
  } else if(s == "plo") {
    return plo;
  } else {
    throw "nonexisting game type";
  }
}

enum game_format {
  ring,
  sitngo,
  tournament
};

string to_string(game_format format){
  if(format == ring){
    return "ring";
  } else if (format == sitngo){
    return "sitngo";
  } else if (format == tournament){
    return "tournament";
  } else {
    throw "Impossible";
  }
}

game_format to_game_format(string s){
  if(s == "ring"){
    return ring;
  } else if(s == "sitngo"){
    return sitngo;
  } else if (s == "tournament"){
    return tournament;
  } else {
    throw "nonexistent game format";
  }
}

string to_string(const vector<int>& array) {
  //no trailing comma
  string s = "";
  s += "[";
  if(array.size() > 0){
    for(int i = 0; i < array.size() - 1; i++){
      s += to_string(array[i]) + ", ";
    }
    s += to_string(array[array.size() - 1]);
  }
  s += "]";
  return s;
}

string to_string(const vector<string>& array) {
  string s = "";
  s += "[";
  if(array.size() > 0){
    for(int i = 0; i < array.size() - 1; i++){
      s += "\"" + array[i] + "\", ";
    }
    s += "\"" + array[array.size() - 1] + "\"";
  }
  s += "]";
  return s;
}

string format_json_raw(string key, string value, int numtab, bool hascomma){
  string s = "";
  for(int i = 0; i < numtab; i++){
    s += "\t";
  }
  s += "\"" + key + "\": " + value;
  if(hascomma){
    s += ",";
  }
  s += "\n";
  return s;
}

string format_json(string key, int value, int numtab, bool hascomma = true){
  return format_json_raw(key, to_string(value), numtab, hascomma);
}

string format_json(string key, string value, int numtab, bool hascomma = true){
  return format_json_raw(key, "\"" + value + "\"", numtab, hascomma);
}

string format_json(string key, const vector<int>& value, int numtab, bool hascomma = true){
  return format_json_raw(key, to_string(value), numtab, hascomma);
}

string format_json(string key, const vector<string>& value, int numtab, bool hascomma = true){
  return format_json_raw(key, to_string(value), numtab, hascomma);
}

string format_json(string key, const set<int>& value, int numtab, bool hascomma = true){
  return format_json(key, vector<int>(value.begin(), value.end()), numtab, hascomma);
}

string format_json(string key, const vector<Card>& value, int numtab, bool hascomma = true){
  vector<string> string_value(value.size());
  for(int i = 0; i < value.size(); i++){
    string_value[i] = value[i].ShowCard();
  }
  return format_json(key, string_value, numtab, hascomma);
}

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

void init_server() {
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
}

string add_to_queue(string session_id, string type, string format, int table_size, int buy_in_or_big_blind) {
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return string("")
          + "{\n"
          + "\t\"error\": \"Player not logged in\""
          + "}\n";
  }
  queue_settings input = {to_game_type(type), to_game_format(format), table_size, buy_in_or_big_blind};
  queue_mutex.lock();
  if (queue.count(input) ){
    vector<string>& array = queue[input];
    if(array.size() == table_size - 1){

    } else {
      array.push_back(player_id);
    }
  } else {

  }
  queue_mutex.unlock();
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
    response = "{\n";
    response += format_json("error", "Game ID Not Found", 1, false);
    response += "}\n";
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
    response = "{\n";
    response += format_json("error", "Table ID Not Found", 1, false);
    response += "}\n";
  }
  all_tables_mutex.unlock_shared();

  return response;
};
