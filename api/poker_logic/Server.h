#ifndef SERVER_H
#include <string>

void init_server();

std::string get_game_from_id(std::string id);
std::string get_table_from_id(std::string id);
std::string add_to_queue(std::string type, std::string format, int table_size, int buy_in_or_big_blind);

#endif
#define SERVER_H
