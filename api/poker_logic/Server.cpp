#include "Server.h"

using namespace std;

using namespace boost::property_tree;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

enum game_type {
  nlhe,
  plo
};

string game_type_toString(game_type type){
  if(type == nlhe){
    return "nlhe";
  } else {
    return "plo";
  }
}

enum game_format {
  ring,
  sitngo,
  tournament
};

string game_format_toString(game_format format){
  if(format == ring){
    return "ring";
  } else if (format == sitngo){
    return "sitngo";
  } else if (format == tournament){
    return "tournament";
  }
}

string json_intarray(const vector<int>& array) {
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

string json_stringarray(const vector<string>& array) {
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

string intvalue(string key, int value, int numtab, bool hascomma){
  string s = "";
  for(int i = 0; i < numtab; i++){
    s += "\t";
  }
  s += "\"" + key + "\": " + to_string(value);
  if(hascomma){
    s += ",";
  }
  s += "\n";
}

string stringvalue(string key, string value, int numtab, bool hascomma){
  value = "\"" + value + "\"";
  string s = "";
  for(int i = 0; i < numtab; i++){
    s += "\t";
  }
  s += "\"" + key + "\": " + value;
  if(hascomma){
    s += ",";
  }
  s += "\n";
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

  string output_game(){
    string s = "{\n";
    s += stringvalue("d", id, 1, true);
    s += stringvalue("type", game_type_toString(type), 1, true);
    s += stringvalue("format", game_format_toString(format), 1, true);
    s += intvalue("table_size", table_size, 1, true);
    s += intvalue("buy_in", buy_in, 1, true);
    s += intvalue("big_blind", big_blind, 1, true);
    s += intvalue("blind_timer", blind_timer, 1, true);
    s += stringvalue("tables", json_stringarray(tables), 1, false);
    s += "}";
  }
};

struct Table {
  string id;
  string game_id;
  vector<string> player_ids;
  HandSimulation hand_sim;

  string output_table(){
    string s = "{\n";
    s += "\t\"id\": \"" + id + "\",\n";
    s += "\t\"game_id\": \"" + game_id + "\",\n";
    s += "\t\"player_ids\": [\n";
    if(player_ids.size() > 0) {
      for(int i = 0; i < player_ids.size() - 1; i++){
        s += "\t\t\"" + player_ids[i] + "\",\n";
      }
      s += "\t\t\"" + player_ids.back() + "\"\n";
    }
    s += "\t],\n";
    vector<Card> board = hand_sim.getBoard();
    vector<string> s_board;
    for(int i = 0; i < board.size(); i++){
      s_board.push_back(board[i].ShowCard());
    }
    stringvalue("board", json_stringarray(s_board), 1, true);
    //variable camelCase to snake_case
    s += "\t\"pots\": " + json_intarray(hand_sim.getPots()) + ",\n";
    s += "\t\"bets\": " + json_intarray(hand_sim.getBets()) + ",\n";
    s += "\t\"stacks\": " + json_intarray(hand_sim.getStacks()) + ",\n";
    s += "\t\"button_location\": " + to_string(hand_sim.getButtonLocation()) + ",\n";
    s += "\t\"current_turn\": " + to_string(hand_sim.getCurrentTurn()) + ",\n";
    s += "\t\"active_players\": "
      + json_intarray(vector<int>(hand_sim.getActivePlayers().begin(), hand_sim.getActivePlayers().end()))
      + ",\n";
    s += "\t\"current_bet\": " + to_string(hand_sim.getCurrentBet()) + ",\n";
    s += "\t\"min_raise\": " + to_string(hand_sim.getMinRaise()) + "\n";
    s += "}\n";
    //tick not implemented yet
    return s;
  }
};

map<string, Game*> all_games;
map<string, Table*> all_tables;
map<string, mutex*> game_mutexes;
map<string, mutex*> table_mutexes;
mutex all_games_mutex;
mutex all_tables_mutex;

int main(){
  cout << "HI2" << endl;
  HttpServer server;
  server.config.port = 8080;

  HandSimulation hs(2, 0, vector<int>(6, 200));
  hs.initBettingRound();
  all_tables["5"] = new Table {
    "5",
    "1234",
    {"a", "b", "c", "d", "e", "f"},
    hs
  };
  table_mutexes["5"] = new mutex;

  server.resource["^/api/queue$"]["POST"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
  //implement table queues
};
  //return table info
  server.resource["^/api/table/([0-9]+)$"]["GET" ] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string table_id = request->path_match[1].str();
    string res;
    all_tables_mutex.lock();
    if(all_tables.count(table_id) > 0){
      table_mutexes[table_id]->lock();
      Table&table = *all_tables[table_id];
      res = table.output_table();
      table_mutexes[table_id]->unlock();
    } else {
      res = "{\n";
      res += "\t\"error\": \"Table ID Not Found\"\n";
      res += "}\n";
    }

    *response << "HTTP/1.1 200 OK\r\n"
              << "Content-Length: " << res.size() << "\r\n\r\n"
              << res;
    all_tables_mutex.unlock();
  };

  server.resource["^/api/game/([0-9]+)$"]["GET" ] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string game_id = request->path_match[1].str();
    string res;
    all_games_mutex.lock();
    if(all_games.count(game_id) > 0){
      game_mutexes[game_id]->lock();
      Game& game = *all_games[game_id];
      res = game.output_game();
      game_mutexes[game_id]->unlock();
    } else {
      res = "{\n";
      res += "\t\"error\": \"Game ID Not Found\"\n";
      res += "}\n";
    }

    *response << "HTTP/1.1 200 OK\r\n"
              << "Content-Length: " << res.size() << "\r\n\r\n"
              << res;
    all_games_mutex.unlock();
  };

  server.start();
}
