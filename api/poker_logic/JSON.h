#include "Card.h"
#include "Server.h"

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
