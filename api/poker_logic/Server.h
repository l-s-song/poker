#ifndef SERVER_H
#include <string>

enum game_type {
  nlhe,
  plo
};

enum game_format {
  ring,
  sitngo,
  tournament
};

void init_server();

std::string get_game_from_id(std::string id);
std::string get_table_from_id(std::string id);
std::string add_to_queue(std::string session_id, std::string type, std::string format, int table_size, int buy_in_or_big_blind);
std::string get_player_id(std::string session_id);
void set_player_id(std::string session_id, std::string player_id);
std::string player_act(std::string session_id, std::string table_id, std::string action, int bet_size);

#endif
#define SERVER_H
