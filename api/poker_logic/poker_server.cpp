#include "client_http.hpp"
#include "server_http.hpp"

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#ifdef HAVE_OPENSSL
#include "crypto.hpp"
#endif

using namespace std;

using namespace boost::property_tree;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

enum game_type {
  nlhe,
  plo
};

enum game_format {
  ring
  sitngo,
  tournament
};

struct Game {
  string id,
  game_type type,
  game_format format,
  int table_size,
  int buy_in,
  int big_blind,
  int blind_timer,
  vector<int> tables
};

struct Table {
  string id,
  int game_id,
  vector<int> player_ids,
  HandSimulation hand_sim,
}

map<string, Game> all_games;
map<string, Table> all_tables;

int main(){
  HttpServer server;
  server.config.port = 8080;

  server.resource["^/api/queue$"]["POST"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
  //implement table queues
  }
  //return table info
  server.resource["^/api/table/([0-9]+)$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string table_id = response->path_match[1].str();
    if(all_tables.contains(table_id)){
      
    }
  }

}
