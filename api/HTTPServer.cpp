#include "HTTPServer.h"
#include "APIFunctions.h"

#include <iostream>

using namespace std;
using namespace boost::property_tree;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

string get_session_id(shared_ptr<HttpServer::Request> request) {
  auto cookie_it = request->header.find("Cookie");
  if (cookie_it == request->header.end()) {
    return "";
  } else {
    string cookie = cookie_it->second;
    bool reading_name = true;
    bool reading_session_id = false;
    string name = "";
    string session_id = "";
    for(int i = 0; i < cookie.size(); i++) {
      char ch = cookie[i];
      if (ch == ';') {
        reading_session_id = false;
        reading_name = true;
        continue;
      }
      if (reading_name) {
        if (ch == '=') {
          reading_name = false;
          if (name == "session_id") {
            reading_session_id = true;
          }
        } else {
          name += cookie[i];
        }
      } else if (reading_session_id) {
        session_id += ch;
      }
    }
    return session_id;
  }
}

void ok(shared_ptr<HttpServer::Response>& response, const string& content) {
  *response << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << content.length() << "\r\n"
            << "\r\n"
            << content;
}

void bad(shared_ptr<HttpServer::Response>& response, const string& error) {
  *response << "HTTP/1.1 400 Bad Request\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << error.size() << "\r\n"
            << "\r\n"
            << error;
}

int main(){
  cout << "Server Starting" << endl;
  HttpServer server;
  server.config.port = 8080;

  init_server();

  server.resource["^/api/games$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string games_json = get_games();

    ok(response, games_json);
  };

  server.resource["^/api/game/([0-9]+)$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string id = request->path_match[1].str();
    string game_json = get_game_from_id(id);

    ok(response, game_json);
  };

  server.resource["^/api/table/([0-9]+)$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string id = request->path_match[1].str();
    string table_json = get_table_from_id(id);

    ok(response, table_json);
  };

  server.resource["^/api/queue$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string queue = get_queues();
    ok(response, queue);
  };

  server.resource["^/api/queue$"]["POST"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string session_id = get_session_id(request);
    try {
      ptree pt;
      read_json(request->content, pt);
      vector<string> types;
      for(auto& type : pt.get_child("types")) {
        types.push_back(type.second.get_value<string>());
      }
      string format = pt.get<string>("format");
      vector<int> table_sizes;
      for(auto& table_size : pt.get_child("table_sizes")) {
        table_sizes.push_back(table_size.second.get_value<int>());
      }
      int buy_in_or_big_blind;
      if (pt.count("buy_in") > 0){
        buy_in_or_big_blind = pt.get<int>("buy_in");
      } else {
        buy_in_or_big_blind = pt.get<int>("big_blind");
      }
      string content = add_to_queue(session_id, types, format, table_sizes, buy_in_or_big_blind);

      ok(response, content);
    } catch(const exception &e) {
      bad(response, "Malformed JSON");
    }
  };

  server.resource["^/api/act"]["POST"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string session_id = get_session_id(request);
    try {
      ptree pt;
      read_json(request->content, pt);
      string table_id = pt.get<string>("table_id");
      string action = pt.get<string>("action");
      int bet_size = pt.get<int>("bet_size");
      string content = player_act(session_id, table_id, action, bet_size);

      ok(response, content);
    } catch(const exception &e) {
      bad(response, "Malformed JSON");
    }
  };

  server.resource["^/api/leave"]["POST"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string session_id = get_session_id(request);
    try {
      ptree pt;
      read_json(request->content, pt);
      string game_id = pt.get<string>("game_id");
      string content = player_leave(session_id, game_id);

      ok(response, content);
    } catch(const exception &e) {
      bad(response, "Malformed JSON");
    }
  };

  server.resource["^/api/login/([0-9]+)$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string session_id = get_session_id(request);
    if (session_id == "") {
      session_id = generate_id();
    }
    string player_id = request->path_match[1].str();
    set_player_id(session_id, player_id);
    string content = "{}";

    *response << "HTTP/1.1 200 OK\r\n"
              << "Set-Cookie: " << "session_id=" << session_id << "; path=/" << "\r\n"
              << "Content-Length: " << content.size() << "\r\n"
              << "\r\n"
              << content;
  };

  server.start();
}
