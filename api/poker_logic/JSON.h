#ifndef JSON_H
#include "Card.h"
#include "Server.h"

string repeat_string(string s, int a){
  //returns string that is string s repeated a times
  string ret = "";
  for(int i = 0; i < a; i++){
    ret += s;
  }
  return ret;
}

string to_string(game_type type){
  if(type == nlhe){
    return "nlhe";
  } else {
    return "plo";
  }
}

bool is_game_type(string s) {
  return s == "nlhe" || s == "plo";
}

bool is_game_format(string s) {
  return s == "ring" || s == "sitngo" || s == "tournament";
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

string error_json(string error){
  return  string("")
        + "{\n"
        + format_json("error", error, 1, false)
        + "}\n";
}

string Game::to_json() {
  return to_json(0);
}

string Game::to_json(int numtab) {
  string s = repeat_string("\t", numtab) + "{\n";
  s += format_json("id", id, 1 + numtab);
  s += format_json("name", name,  1 + numtab);
  s += format_json("type", to_string(settings.type), 1 + numtab);
  s += format_json("format", to_string(settings.format), 1 + numtab);
  s += format_json("table_size", settings.table_size, 1 + numtab);
  if (settings.format == ring) {
    s += format_json("big_blind", settings.big_blind, 1 + numtab);
  } else {
    s += format_json("buy_in", settings.buy_in, 1 + numtab);
    //s += format_json("big_blind", blind_timer, 1 + numtab);
    //s += format_json("blind_timer", blind_timer, 1 + numtab);
  }
  s += format_json("num_players", player_ids.size() + waiting_players.size(), 1 + numtab);
  s += format_json("tables", table_ids, 1 + numtab, false);
  s += repeat_string("\t", numtab) + "}";
  return s;
}

string Table::to_json() {
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
  s += "}";
  return s;
};

#endif
#define JSON_H
