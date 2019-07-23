#include "HTTPServer.h"
#include "Server.h"

#include <iostream>

using namespace std;
using namespace boost::property_tree;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

int main(){
  cout << "Server Starting" << endl;
  HttpServer server;
  server.config.port = 8080;

  init_server();

  server.resource["^/api/game/([0-9]+)$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string id = request->path_match[1].str();
    string game_json = get_game_from_id(id);

    *response << "HTTP/1.1 200 OK\r\n"
              << "Content-Length: " << game_json.size() << "\r\n\r\n"
              << game_json;
  };

  server.resource["^/api/table/([0-9]+)$"]["GET"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    string id = request->path_match[1].str();
    string table_json = get_table_from_id(id);

    *response << "HTTP/1.1 200 OK\r\n"
              << "Content-Length: " << table_json.size() << "\r\n\r\n"
              << table_json;
  };

  server.resource["^/api/queue$"]["POST"] = [](
    shared_ptr<HttpServer::Response> response,
    shared_ptr<HttpServer::Request> request
  ) {
    try {
      ptree pt;
      read_json(request->content, pt);
      string type = pt.get<string>("type");
      string format = pt.get<string>("format");
      int table_size = pt.get<int>("table_size");
      int buy_in_or_big_blind;
      if (pt.count("buy_in") > 0){
        buy_in_or_big_blind = pt.get<int>("buy_in");
      } else {
        buy_in_or_big_blind = pt.get<int>("big_blind");
      }
      string content = add_to_queue(type, format, table_size, buy_in_or_big_blind);

      *response << "HTTP/1.1 200 OK\r\n"
                << "Content-Length: " << content.length() << "\r\n\r\n"
                << content;
    }
    catch(const exception &e) {
      *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
                << e.what();
    }
  }

  server.start();
}
